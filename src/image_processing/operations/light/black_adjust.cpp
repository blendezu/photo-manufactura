#include "black_adjust.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

#include "../../core/halide_build_graph.h"
#include "color_space.h"
#include "halide_color_space.h"
#include "image_utils.h"

void AdjustBlack::prepareParameters(const cv::Mat& srcImg) {
    // 1. Check if the Input Image is empty
    if (srcImg.empty()) {
        std::cerr << "[AdjustBlack] ❌ Error: The Input Image is empty\n";
        return;
    }

    // 2. Create a 512x512 thumbnail for Calculate min max to improve performance
    cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);

    // 3. Find Min Max
    float minL;
    float maxL;

    // --- Path A. Color Image ---
    if (srcImg.channels() == 3) {
        // 1. Convert the Thumbnail to HSL
        cv::Mat hslThumb = ColorSpace::convertBGR2HSL(thumbnail);

        // 2. Find the min max
        auto minMaxL = ImageUtils::calculateMinMax(hslThumb, 2);  // Channel 2 is Luminance
        minL = std::get<0>(minMaxL);
        maxL = std::get<1>(minMaxL);
    }
    // --- Path B. Gray Image ---
    else {
        auto minMaxVal = ImageUtils::calculateMinMax(thumbnail, 0);
        minL = std::get<0>(minMaxVal);
        maxL = std::get<1>(minMaxVal);
    }

    // 4. Find maxRange based on Bit Depth
    float maxRange = (srcImg.depth() == CV_8U) ? 255.0f : 65535.0f;

    // 5. Calculate the lower and upper point for black area
    float range = maxL - minL;
    float lowerPoint = minL + range * LOWER_THRESHOLD_PERCENT;
    float upperPoint = minL + range * UPPER_THRESHOLD_PERCENT;

    // 6. Calculate the blackFactor
    float blackFactor = m_black / BLACK_SCALING_FACTOR;

    // 7. Update Halide Runtime Parameters
    p_blackFactor.set(blackFactor);
    p_lowerPoint.set(lowerPoint);
    p_upperPoint.set(upperPoint);
    p_maxRange.set(maxRange);
}

Halide::Func AdjustBlack::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                                     Halide::Var c) {
    // Inverse maxRange to avoid Division later
    Halide::Expr invMaxRange = Halide::select(p_maxRange < 10e-6f, 0.0f, 1.0f / p_maxRange);

    // --- Path A. Gray Image ---
    if (srcImg.dimensions() == 2) {
        // 1. Extract the current Value and normalize it
        Halide::Expr currVal = srcImg(x, y) * invMaxRange;

        // 2. Calculate the new Value using Shared Logic
        Halide::Expr newVal =
            HalideBuildGraph::apply_black_L(currVal, p_blackFactor, p_lowerPoint, p_upperPoint);

        // 5. Denormalize the new Value and assign ist to Destination Image
        Halide::Func dstImg("black_adjust_gray_image");
        dstImg(x, y) = newVal * p_maxRange;

        return dstImg;
    }

    // --- Path B. Color Image ---
    // 1. Extract BGR Values and normalize them
    Halide::Expr B = srcImg(x, y, 0) * invMaxRange;
    Halide::Expr G = srcImg(x, y, 1) * invMaxRange;
    Halide::Expr R = srcImg(x, y, 2) * invMaxRange;

    // 2. Convert to HSL and extract HSL Values
    std::vector<Halide::Expr> hslImg = HalideColorSpace::BGR2HSL(B, G, R);
    Halide::Expr H = hslImg[0];
    Halide::Expr S = hslImg[1];
    Halide::Expr currL = hslImg[2];

    // 3. Calculate the new Luminace Value using Shared Logic
    Halide::Expr newL =
        HalideBuildGraph::apply_black_L(currL, p_blackFactor, p_lowerPoint, p_upperPoint);

    // 6. Convert back to BGR
    std::vector<Halide::Expr> bgrImg = HalideColorSpace::HSL2BGR(H, S, newL);

    // 7. Channel Value Selection
    Halide::Expr val =
        Halide::select(c == 0, bgrImg[0], Halide::select(c == 1, bgrImg[1], bgrImg[2]));

    // 8. Denormalize the new Values and assign to the Destination Image
    Halide::Func dstImg("black_adjust_color_image");
    dstImg(x, y, c) = val * p_maxRange;

    return dstImg;
}

cv::Mat AdjustBlack::apply(const cv::Mat& srcImg) {
    // Check if the source Image is empty
    if (srcImg.empty()) {
        std::cerr << "Error in AdjustBlack: empty input image\n";
        return cv::Mat();
    }

    // Calculate the Change Factor
    float blackFactor = m_black / BLACK_SCALING_FACTOR;

    // --- Path A. 8-Bit Gray Image
    if (srcImg.type() == CV_8UC1) {
        return blackGrayImgTemplate<uchar>(srcImg, blackFactor);
    }

    // --- Path B. 16-Bit Gray Image
    else if (srcImg.type() == CV_16UC1) {
        return blackGrayImgTemplate<ushort>(srcImg, blackFactor);
    }

    // --- Path C. 8/16 Bit Color Image ---
    else if (srcImg.type() == CV_16UC3 || srcImg.type() == CV_8UC3) {
        // 1. Convert the Source Image to HSL
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        // 2. Find min max on a 512x512 thumbnail
        cv::Mat thumbnail = ImageUtils::createThumbnail(hslImg);
        auto minMaxVal =
            ImageUtils::calculateMinMax(thumbnail, 2);  // thumbnail is already an HSL Image
        float minL = std::get<0>(minMaxVal);
        float maxL = std::get<1>(minMaxVal);

        // 3. Precalculate Parameters for Weight Calculation later
        auto weightParams = ImageUtils::precalculateDarkWeightParams(minL, maxL, 0.1f, 0.3f);

        // 4. Calculate the length of 1D Array
        int len = hslImg.cols * 3;

        // clang-format off
        // 5. Iteration through the Image using OpenMP for Parallelism
        // Using HSL Image
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < hslImg.rows; y++) {
            // 5.1 Get the pointer of first pixel of the line
            float* __restrict hslPtr = hslImg.ptr<float>(y);

            // 5.2 Caculate pixel-wise
            for (int x = 2; x < len; x += 3) {
                // 5.2.1 Extract the current Luminance Value
                float currL = hslPtr[x];

                // 5.2.2 Calculate the weight
                float weight = ImageUtils::calculateDarkWeight(currL, weightParams);

                // 5.2.3 Calculate the new Luminance Value and clamp it
                float newL = currL + weight * blackFactor;
                newL = std::clamp(newL, 0.0f, 1.0f);

                // 5.2.4 Assign the new Luminance Value
                hslPtr[x] = newL;
            }
        }
        // 6. Convert back to BGR Image with original Bit Depth
        if (srcImg.depth() == CV_16U)
            return ColorSpace::convertHSL2BGR(hslImg, 16);
        else {
            return ColorSpace::convertHSL2BGR(hslImg, 8);
        }
    }

    // --- Path D. Unsupported Image Type ---
    else {
        std::cerr << "Error in AdjustBlack: unsupported image type\n";
        return cv::Mat();
    }
}