#include "brightness_adjust.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <opencv2/opencv.hpp>
#include <vector>

#include "../../core/halide_build_graph.h"
#include "../../utils/image_utils.h"
#include "color_space.h"
#include "halide_color_space.h"

// --- Calculate parameters for buildGraph on CPU ---
void AdjustBrightness::prepareParameters(const cv::Mat& srcImg) {
    if (srcImg.empty())
        return;

    // 1. Calculate Min/Max Luminance
    float minL = 0.0f;
    float maxL = 1.0f;

    // Use thumbnail for speed
    cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);

    if (srcImg.channels() == 3) {
        cv::Mat hslThumbnail = ColorSpace::convertBGR2HSL(thumbnail);
        auto minMax = ImageUtils::calculateMinMax(hslThumbnail, 2);
        minL = std::get<0>(minMax);
        maxL = std::get<1>(minMax);
    } else {
        auto minMax = ImageUtils::calculateMinMax(thumbnail, 0);
        minL = std::get<0>(minMax);
        maxL = std::get<1>(minMax);
    }

    // 2. Set Max Range based on Depth
    if (srcImg.depth() == CV_16U) {
        p_maxRange.set(65535.0f);
        p_depthScale.set(256.0f);
    } else {
        p_maxRange.set(255.0f);
        p_depthScale.set(1.0f);
    }

    // 3. Calculate Factors
    // Factor ranges roughly -1.0 to 1.0
    float changeFactor = static_cast<float>(m_brightness) / BRIGHTNESS_SCALING_FACTOR;
    p_changeFactor.set(changeFactor);

    p_minL.set(minL);
    p_maxL.set(maxL);

    // Note: p_brightness is legacy name, reused here if needed but changeFactor is primary
    p_brightness.set((float)m_brightness);
}

// --- Define the Halide Computation Graph ---
Halide::Func AdjustBrightness::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                                          Halide::Var c) {
    // --- Path A. Gray Image ---
    if (srcImg.dimensions() == 2) {
        // 1. Get current Value (normalized to 0..1 by caller usually? No, srcImg is raw)
        // Assume srcImg is roughly in p_maxRange scale.

        Halide::Expr invMaxRange = 1.0f / p_maxRange;
        Halide::Expr currVal = srcImg(x, y) * invMaxRange;

        // 2. Calculate newly brightness adjusted value
        Halide::Expr newVal =
            HalideBuildGraph::apply_brightness_L(currVal, p_changeFactor, p_minL, p_maxL);

        // 3. Assign and denormalize
        Halide::Func dstImg("brightness_adjust_gray_image");
        dstImg(x, y) = newVal * p_maxRange;
        return dstImg;
    }

    // --- Path B. Color Image ---

    // 1. Inverse maxRange to avoid Division
    Halide::Expr invMaxRange = 1.0f / p_maxRange;

    // 2. Extract the BGR Values
    Halide::Expr B = srcImg(x, y, 0) * invMaxRange;
    Halide::Expr G = srcImg(x, y, 1) * invMaxRange;
    Halide::Expr R = srcImg(x, y, 2) * invMaxRange;

    // 3. Convert to HSL
    std::vector<Halide::Expr> hslImg = HalideColorSpace::BGR2HSL(B, G, R);

    // 4. Extract HSL Values
    Halide::Expr H = hslImg[0];
    Halide::Expr S = hslImg[1];
    Halide::Expr currL = hslImg[2];

    // 5. Calculate new Luminance Value and clamp it
    Halide::Expr newL = HalideBuildGraph::apply_brightness_L(currL, p_changeFactor, p_minL, p_maxL);

    // 6. Convert back to BGR
    std::vector<Halide::Expr> bgrImg = HalideColorSpace::HSL2BGR(H, S, newL);

    // 7. Value Selection
    Halide::Expr val =
        Halide::select(c == 0, bgrImg[0], Halide::select(c == 1, bgrImg[1], bgrImg[2]));

    // 8. Assign new Values to the Destination Image and denormalize back to original Bit Depth
    Halide::Func dstImg("brightness_adjust_color_image");
    dstImg(x, y, c) = val * p_maxRange;

    return dstImg;
}

cv::Mat AdjustBrightness::apply(const cv::Mat& srcImg) {
    // Check if the input image is empty
    if (srcImg.empty()) {
        std::cerr << "Error in BrightnessAdjust: the input image is empty\n";
        return cv::Mat();
    }

    // --- A. Color Image ---
    if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        // 1. Calculate Min/Max (Similar to prepareParameters)
        cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);
        cv::Mat hslThumbnail = ColorSpace::convertBGR2HSL(thumbnail);
        auto minMax = ImageUtils::calculateMinMax(hslThumbnail, 2);
        float minL = std::get<0>(minMax);
        float maxL = std::get<1>(minMax);
        float invRange = 1.0f / (maxL - minL + 0.0001f);

        // 2. Convert BGR to HSL
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        // 3. Calculate the factor
        // Factor is roughly -1 to 1 based on slider
        const float changeFactor = static_cast<float>(m_brightness) / BRIGHTNESS_SCALING_FACTOR;

        // clang-format off
        // 4. Parallelizing across lines direct the hslImg instead a temporary image to save Memory
        #pragma omp parallel for
        // clang-format on

        for (int y = 0; y < hslImg.rows; y++) {
            // 2.1 Get the pointer of the first pixel each line
            float* __restrict ptr = hslImg.ptr<float>(y);

            // 2.2 Calculate the length of 1D Array
            int len = hslImg.cols * 3;

            // 2.3 Calculate pixel-wise
            for (int i = 2; i < len; i += 3) {  // Only L-Chanel (Index 2, 5, 8...)
                // 2.3.1 Extract the current Luminance
                float currL = ptr[i];

                // 2.3.2 Calculate the new Luminance
                // Midtone Curve Logic
                // Normalize L relative to image range
                float l_norm = (currL - minL) * invRange;

                // Curve: 4 * x * (1 - x)
                float weight = 4.0f * l_norm * (1.0f - l_norm);
                if (weight < 0.0f)
                    weight = 0.0f;  // Handle out of bounds if simple normalization

                float delta = weight * changeFactor;
                float newVal = std::clamp(currL + delta, 0.0f, 1.0f);

                // 2.3.4 Assign the new Luminance to the hsl Image
                ptr[i] = newVal;
            }
        }

        // 3. convert to BGR image
        int depth = (srcImg.type() == CV_8UC3) ? 8 : 16;
        return ColorSpace::convertHSL2BGR(hslImg, depth);
    }

    // --- Path B. 8-bit or 16-bit Gray Image ---
    else if (srcImg.type() == CV_8UC1 || srcImg.type() == CV_16UC1) {
        // Calc min/max for gray
        cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);
        auto minMax = ImageUtils::calculateMinMax(thumbnail, 0);
        float minL = std::get<0>(minMax);
        float maxL = std::get<1>(minMax);

        float changeFactor = static_cast<float>(m_brightness) / BRIGHTNESS_SCALING_FACTOR;

        if (srcImg.type() == CV_8UC1)
            return grayImgTemplate<uchar>(srcImg, changeFactor, minL, maxL);
        else
            return grayImgTemplate<ushort>(srcImg, changeFactor, minL, maxL);
    }

    // --- Path D. Unsupported Image Type
    else {
        std::cerr << "Error: unsupported image type\n";
        return cv::Mat();
    }
}