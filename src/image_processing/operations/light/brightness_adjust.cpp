#include "brightness_adjust.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <opencv2/opencv.hpp>
#include <vector>

#include "color_space.h"
#include "halide_color_space.h"
#include "image_algorithms.h"

// --- Calculate parameters for buildGraph on CPU ---
void AdjustBrightness::prepareParameters(const cv::Mat& srcImg) {
    // 1. Determine the maxRange Value based on the Bit Depth
    if (srcImg.depth() == CV_16U) {
        p_maxRange.set(65535.0f);
        p_depthScale.set(256.0f);
    }

    else {
        p_maxRange.set(255.0f);
        p_depthScale.set(1.0f);
    }

    // 2. Calculate the changeFactor
    float changeFactor = 1.0f + m_brightness / BRIGHTNESS_SCALING_FACTOR;
    p_changeFactor.set(changeFactor);

    // 3. Assign the brightness changeValue
    p_brightness.set((float)m_brightness);
}

// --- Define the Halide Computation Graph ---
Halide::Func AdjustBrightness::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                                          Halide::Var c) {
    // --- Path A. Gray Image ---
    if (srcImg.dimensions() == 2) {
        // 1. Get the current Value
        Halide::Expr currVal = srcImg(x, y);

        // 2. Calculate the new Value
        Halide::Expr newVal = currVal + p_brightness * p_depthScale;

        // 3. Clamp the new value
        newVal = Halide::clamp(newVal, 0.0f, p_maxRange);

        // 4. Assign the new Value to the Destination Image
        Halide::Func dstImg("brightness_adjust_gray_image");
        dstImg(x, y) = newVal;
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
    Halide::Expr newL = ImageAlgorithms::apply_brightness(currL, p_changeFactor);

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
        // 1. Convert BGR to HSL
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        // 2. Calculate the multiplier
        const float changeFactor = 1.0f + m_brightness / BRIGHTNESS_SCALING_FACTOR;

        // clang-format off
        // 2. Parallelizing across lines direct the hslImg instead a temporary image to save Memory
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
                float newVal = currL * changeFactor;

                // 2.3.3 Clamp the Value
                newVal = std::clamp(newVal, 0.0f, 1.0f);

                // 2.3.4 Assign the new Luminance to the hsl Image
                ptr[i] = newVal;
            }
        }

        // 3. convert to BGR image
        int depth = (srcImg.type() == CV_8UC3) ? 8 : 16;
        return ColorSpace::convertHSL2BGR(hslImg, depth);
    }

    // --- Path B. 8-bit Gray Image ---
    else if (srcImg.type() == CV_8UC1) {
        return grayImgTemplate<uchar>(srcImg, m_brightness);
    }

    // --- Path C. 16-bit Gray Image ---
    else if (srcImg.type() == CV_16UC1) {
        return grayImgTemplate<ushort>(srcImg, m_brightness);
    }

    // --- Path D. Unsupported Image Type
    else {
        std::cerr << "Error: unsupported image type\n";
        return cv::Mat();
    }
}