#include "shadow_adjust.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <cstdint>
#include <iostream>
#include <opencv2/core/mat.hpp>

#include "../../core/halide_build_graph.h"
#include "color_space.h"
#include "halide_color_space.h"
#include "halide_image_utils.h"
#include "image_utils.h"

// ===============================================================
// I. Prepare parameters on CPU for GPU
// ===============================================================

void AdjustShadow::prepareParameters(const cv::Mat& srcImg) {
    // 1. Check if the input image is empty
    if (srcImg.empty()) {
        std::cerr << "[AdjustShadow] Error: The input image is empty\n";
        return;
    }

    // --- 2. Find the min/max value (Statistic) ---
    // 2.1. Create thumbnail 512x512 and calculate min/max value on it to improve performance
    cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);

    // 2.2. Calculate min/max value if Color Image
    float minL = 0.0f, maxL = 1.0f;
    if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        // Convert the thumbnail to HSL before calculate min/max value because of Color Image
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(thumbnail);
        auto minMaxVal = ImageUtils::calculateMinMax(hslImg, 2);  // Channel 2 is Luminance

        // Update the min/max value into cache
        minL = std::get<0>(minMaxVal);
        maxL = std::get<1>(minMaxVal);
    }

    // 2.3. Calculate min/max value if Gray Image
    else {
        auto minMaxVal = ImageUtils::calculateMinMax(thumbnail, 0);
        minL = std::get<0>(minMaxVal);
        maxL = std::get<1>(minMaxVal);
    }

    // --- 3. Calculate Logic Parameters ---
    float range = maxL - minL;

    // 3.1. Determine the dynamic thresholds based on the image's actual dynamic range
    float underVal = minL + (range * WEIGHT_RANGE_LOWER);
    float upperVal = minL + (range * WEIGHT_RANGE_UPPER);

    float changeFactor = m_shadow / SHADOW_SCALING_FACTOR;
    float maxRange = (srcImg.depth() == CV_8U) ? 255.0f : 65535.0f;

    // 3.2 Update Halide Runtime Parameters
    p_shadowFactor.set(changeFactor);
    p_underVal.set(underVal);
    p_upperVal.set(upperVal);
    p_maxRange.set(maxRange);
}

// ===============================================================
// II. Build Graph (Halide - GPU)
// ===============================================================

Halide::Func AdjustShadow::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                                      Halide::Var c) {
    // 1. Inversed maxRange to avoid division later
    Halide::Expr invMaxRange = 1 / p_maxRange;

    // 2. Gray Image
    if (srcImg.dimensions() == 2) {  // Gray Image has only 2 dimensions (x, y)

        // 2.1. Calculate the current pixel value
        Halide::Expr currVal = srcImg(x, y) * invMaxRange;

        // 2.2 Calculate the weight
        Halide::Expr weight =
            HalideImageUtils::calculateDarkWeight(currVal, p_underVal, p_upperVal);

        // 2.3 Calculate the delta Value
        Halide::Expr deltaVal = weight * p_shadowFactor;

        // 2.4 Calculate the new pixel value
        Halide::Expr newVal = Halide::clamp(currVal + deltaVal, 0.0f, 1.0f);

        // 2.5 Convert back to original bit depth
        Halide::Func dstImg("adjust_shadow_gray_image");
        dstImg(x, y) = newVal * p_maxRange;

        return dstImg;
    }

    // 3. Color Image

    // 3.1 Channel Splitting & HSL Conversion
    Halide::Expr B = srcImg(x, y, 0) * invMaxRange;
    Halide::Expr G = srcImg(x, y, 1) * invMaxRange;
    Halide::Expr R = srcImg(x, y, 2) * invMaxRange;

    // 3.2 Convert to HSL
    std::vector<Halide::Expr> hslImg = HalideColorSpace::BGR2HSL(B, G, R);

    // 3.3 Extract current Luminance
    Halide::Expr currL = hslImg[2];

    // 3.4 Weight Calculation
    Halide::Expr weight = HalideImageUtils::calculateDarkWeight(currL, p_underVal, p_upperVal);

    // 3.5 Apply Adjustment
    Halide::Expr deltaL = weight * p_shadowFactor;
    Halide::Expr newL = Halide::clamp(currL + deltaL, 0.0f, 1.0f);

    // 3.6 Convert back to BGR
    std::vector<Halide::Expr> bgrImg = HalideColorSpace::HSL2BGR(hslImg[0], hslImg[1], newL);

    // 3.7 Channel Selection (Re-interleave)
    Halide::Expr val =
        Halide::select(c == 0, bgrImg[0], Halide::select(c == 1, bgrImg[1], bgrImg[2]));

    // 3.8 Denormalize to original bit depth image
    Halide::Func dstImg("adjust_shadow_color_image");
    dstImg(x, y, c) = val * p_maxRange;

    return dstImg;
}

// ===============================================================
// 3. CPU Implementation
// ===============================================================

cv::Mat AdjustShadow::apply(const cv::Mat& srcImg) {
    // 1. Check if the image is empty
    if (srcImg.empty()) {
        std::cerr << "Error in AjustShadow: empty iput image\n";
        return cv::Mat();
    }

    // 2. Calculate the Change Factor
    float shadowFactor = m_shadow / SHADOW_SCALING_FACTOR;

    // 3. For 8- or 16-bit Color Image
    if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        // 3.1 Convert to HSL image
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        // 3.2 Create a thumbnail image and find min max on that to improve image
        cv::Mat thumbnail = ImageUtils::createThumbnail(hslImg);
        auto minMaxVal = ImageUtils::calculateMinMax(thumbnail, 2);
        float minL = std::get<0>(minMaxVal);
        float maxL = std::get<1>(minMaxVal);

        // 3.2 Calculate Parameters for later
        auto weightParams = ImageUtils::precalculateDarkWeightParams(minL, maxL, WEIGHT_RANGE_LOWER,
                                                                     WEIGHT_RANGE_UPPER);

        // 3.3 Calculate the length of 1D Array
        int len = hslImg.cols * 3;

        // clang-format off
        // 3.4 Iteration through the image with OpenMP for parallelism
        #pragma omp parallel for
        // clang-format on

        for (int y = 0; y < hslImg.rows; y++) {
            // 3.4.1 Get the Pointer of the first pixel of each line
            float* __restrict hslPtr = hslImg.ptr<float>(y);

            // 3.4.2 Execute for Luminance Channel
            // Channel 2 is for Luminance so we go 2, 5, 8, ...
            for (int x = 2; x < len; x += 3) {
                // 3.4.2.1 Extract the current Luminance
                float currL = hslPtr[x];

                // 3.4.2.2 Calculate the weight for each Luminance value
                float weight = ImageUtils::calculateDarkWeight(currL, weightParams);

                // 3.4.2.3 Calculate the delta Luminance
                float deltaL = weight * shadowFactor;

                // 3.4.2.4 Calculate the new Luminance
                float newL = currL + deltaL;

                // 3.4.2.5 Clamp the new Luminance for safety
                newL = std::clamp(newL, 0.0f, 1.0f);

                // 3.4.2.6 Assign the new Luminance
                hslPtr[x] = newL;
            }
        }

        // 3.5 Convert back to BGR Image with original Bit Depth
        if (srcImg.type() == CV_8UC3) {
            return ColorSpace::convertHSL2BGR(hslImg, 8);
        } else {
            return ColorSpace::convertHSL2BGR(hslImg, 16);
        }
    }

    // 4. For 8-bit gray image
    else if (srcImg.type() == CV_8UC1) {
        return shadowGrayImgTemplate<uint8_t>(srcImg, shadowFactor);
    }

    // 5. For 16-bit color image
    else if (srcImg.type() == CV_16UC1) {
        return shadowGrayImgTemplate<uint16_t>(srcImg, shadowFactor);
    }

    // 6. For unsupported image type
    else {
        return cv::Mat();
    }
}