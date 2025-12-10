#include "white_adjust.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <iostream>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "color_space.h"
#include "halide_color_space.h"
#include "halide_image_utils.h"
#include "image_utils.h"

// ===================================================================
// 1. Prepare Parameters on CPU for GPU
// ===================================================================

void AdjustWhite::prepareParameters(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "[AdjustWhite] Error: The input image is empty\n";
        return;
    }

    // --- A. Find the Min & Max Value (Statistic) ---

    // 1. Find min/max in a 512x 512 thumbnail image for better performance
    cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);

    // Since 'requiresFreshStats' is true, need to calculate new min/max
    // srcImg contains the result of all previous operations.
    // 2. Calculate min/max if Color Image
    float minL = 0.0f;
    float maxL = 1.0f;
    if (srcImg.channels() == 3) {
        // Convert to HSL before calculate min/max because of Color Image
        cv::Mat hslThumbnail = ColorSpace::convertBGR2HSL(thumbnail);
        auto minMax = ImageUtils::calculateMinMax(hslThumbnail, 2);  // channel 2 is Luminance
        minL = std::get<0>(minMax);
        maxL = std::get<1>(minMax);
    }
    // 3.Calculate min/max if gray image
    else {
        // Don't need to convert to HSL because of Gray Image
        auto minMax = ImageUtils::calculateMinMax(thumbnail, 0);
        minL = std::get<0>(minMax);
        maxL = std::get<1>(minMax);
    }

    // --- B. Calculate Logic Parameters ---
    float range = maxL - minL;

    // 3. Determine the dynamic thresholds based on the image's actual dynamic range
    float underVal = minL + (range * WEIGHT_RANGE_LOWER);
    float upperVal = minL + (range * WEIGHT_RANGE_UPPER);

    float factor = static_cast<float>(m_white) / WHITE_SCALING_FACTOR;
    float maxRange = (srcImg.depth() == CV_8U) ? 255.0f : 65535.0f;

    // --- C. Update Halide Runtime Parameters ---
    p_underVal.set(underVal);
    p_upperVal.set(upperVal);
    p_whiteFactor.set(factor);
    p_maxRange.set(maxRange);
}

// ===================================================================
// 2. Build Graph (Halide - GPU)
// ===================================================================

Halide::Func AdjustWhite::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                                     Halide::Var c) {
    // Inversed max range to avoid Division later
    Halide::Expr invMaxRange = 1.0f / p_maxRange;

    // --- Path A. Gray image ---
    if (srcImg.dimensions() == 2) {  // Gray image has only 2 dimensions (x, y)
        Halide::Expr currVal = srcImg(x, y) * invMaxRange;
        Halide::Expr weight =
            HalideImageUtils::calculateBrightWeight(currVal, p_underVal, p_upperVal);
        Halide::Expr deltaVal = weight * p_whiteFactor;
        Halide::Expr newVal = Halide::clamp(currVal + deltaVal, 0.0f, 1.0f);

        Halide::Func dstImg("adjust_white_gray_image");

        // Convert back to original Bit Depth
        dstImg(x, y) = newVal * p_maxRange;
        return dstImg;
    }
    // --- Path B. Color Image ---

    // 1. Channel Splitting & HSL Conversion
    Halide::Expr B = srcImg(x, y, 0) * invMaxRange;
    Halide::Expr G = srcImg(x, y, 1) * invMaxRange;
    Halide::Expr R = srcImg(x, y, 2) * invMaxRange;

    // 2. Convert to HSL
    std::vector<Halide::Expr> hslImg = HalideColorSpace::BGR2HSL(B, G, R);
    Halide::Expr currL = hslImg[2];  // Current Luminance

    // 3. Weight Calculation
    Halide::Expr weight = HalideImageUtils::calculateBrightWeight(currL, p_underVal, p_upperVal);

    // 4. Apply Adjustment
    Halide::Expr deltaL = weight * p_whiteFactor;
    Halide::Expr newL = Halide::clamp(currL + deltaL, 0.0f, 1.0f);

    // 5. Convert back to BGR
    std::vector<Halide::Expr> bgr = HalideColorSpace::HSL2BGR(hslImg[0], hslImg[1], newL);

    // 6. Channel Selection (Re-interleave)
    Halide::Expr outColor = Halide::select(c == 0, bgr[0], Halide::select(c == 1, bgr[1], bgr[2]));

    // 7. Denormalize to original bit depth image
    Halide::Func dstImg("adjust_white_image");  // output image
    dstImg(x, y, c) = outColor * p_maxRange;
    return dstImg;
}

// ===================================================================
// 3. CPU Implementation
// ===================================================================
cv::Mat AdjustWhite::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "[AdjustWhite] Error: Input image is empty\n";
        return cv::Mat();
    }

    // 1. Prepare Parameters
    // Normalize user input (-100 -> 100) to internal factor
    float changeFactor = static_cast<float>(m_white) / WHITE_SCALING_FACTOR;

    // --- Path A. Color Images ---
    if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        // 1. Statistics Calculation on 512x512 thumbnail image for better performance
        cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);
        cv::Mat hslThumbnail = ColorSpace::convertBGR2HSL(thumbnail);

        auto minMaxVal = ImageUtils::calculateMinMax(hslThumbnail, 2);
        float minL = std::get<0>(minMaxVal);
        float maxL = std::get<1>(minMaxVal);

        // 2. Pre-calculate weighting logic
        auto weightParams = ImageUtils::precalculateWhiteWeightParams(
            minL, maxL, WEIGHT_RANGE_LOWER, WEIGHT_RANGE_UPPER);

        // 3. Convert to HSL
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        // clang-format off
        // 4. Parallel Pixel Processing
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < hslImg.rows; y++) {
            float* __restrict hslPtr = hslImg.ptr<float>(y);

            int len = hslImg.cols * 3;  // Dimension for 1D-Arry

            for (int x = 2; x < len; x += 3) {  // Start at 2 and increment 3 because H S L H S L
                float currL = hslPtr[x];

                float weight = ImageUtils::calculateBrightWeight(currL, weightParams);

                float deltaL = weight * changeFactor;
                float newL = std::clamp(currL + deltaL, 0.0f, 1.0f);

                hslPtr[x] = newL;
            }
        }

        // 5. Convert back to BGR
        if (srcImg.depth() == CV_8U) {
            return ColorSpace::convertHSL2BGR(hslImg, 8);
        } else {
            return ColorSpace::convertHSL2BGR(hslImg, 16);
        }
    }

    else if (srcImg.type() == CV_8UC1) {
        return whiteGrayImgTemplate<uint8_t>(srcImg, changeFactor);
    }

    else if (srcImg.type() == CV_16UC1) {
        return whiteGrayImgTemplate<uint16_t>(srcImg, changeFactor);
    }

    else {
        std::cerr << "[AdjustWhite] Error: Unsupported image type\n";
        return cv::Mat();
    }
}