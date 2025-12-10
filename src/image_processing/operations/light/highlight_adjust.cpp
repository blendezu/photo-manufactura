#include "highlight_adjust.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <cstdint>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>

#include "color_space.h"
#include "halide_color_space.h"
#include "halide_image_utils.h"
#include "image_utils.h"

// --- A. Calculate Statistic, if required ---
void AdjustHighlight::prepareParameters(const cv::Mat& srcImg) {
    // 1. Check if the input image is empty
    if (srcImg.empty()) {
        std::cerr << "[AdjustHighlight] Error: The input image is empty\n";
        return;
    }

    // 2. Find min/max on the 512x512 thumbnail for better performance
    cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);
    float minL = 0.0f;
    float maxL = 1.0f;

    // --- Path A. For Color Image ---
    if (srcImg.channels() == 3) {
        // 1. Convert the thumbnail to HSL image
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(thumbnail);

        // 2. Find the min max on this thumbnail
        auto minMaxVal = ImageUtils::calculateMinMax(hslImg, 2);  // Channel 2 is Luminance
        minL = std::get<0>(minMaxVal);
        maxL = std::get<1>(minMaxVal);
    }

    // --- Path B. For Gray Image ---
    else {
        auto minMaxVal = ImageUtils::calculateMinMax(thumbnail, 0);
        minL = std::get<0>(minMaxVal);
        maxL = std::get<1>(minMaxVal);
    }

    // 3. Calculate the under & upper thresholds
    float range = maxL - minL;
    float underVal = minL + (range * WEIGHT_RANGE_LOWER);
    float upperVal = minL + (range * WEIGHT_RANGE_UPPER);

    float factor = static_cast<float>(m_highlight) / HIGHLIGHT_SCALING_FACTOR;
    float maxRange = (srcImg.depth() == CV_8U) ? 255.0f : 65535.0f;

    // 4. Update Halide Runtime Parameters
    p_underVal.set(underVal);
    p_upperVal.set(upperVal);
    p_maxRange.set(maxRange);
    p_highlightFactor.set(factor);
}

// --- Define the Halide Build Graph ---
Halide::Func AdjustHighlight::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                                         Halide::Var c) {
    // 1. Inverse Max Range to avoid Division later
    Halide::Expr invMaxRange = 1.0f / p_maxRange;

    // --- Path A. Gray Image --
    if (srcImg.dimensions() == 2) {
        // 1. Extract the current Luminance Value
        Halide::Expr currVal = srcImg(x, y) * invMaxRange;

        // 2. Calculate the weight
        Halide::Expr weight =
            HalideImageUtils::calculateBrightWeight(currVal, p_underVal, p_upperVal);

        // 3. Calculate the delta Value
        Halide::Expr deltaVal = weight * p_highlightFactor;

        // 4. Calculate the new Value and clamp it
        Halide::Expr newVal = Halide::clamp(deltaVal + currVal, 0.0f, 1.0f);

        // 5. Assign the new Value to Destination Image & convert back to original Bit Depth
        Halide::Func dstImg("adjust_highlight_gray_image");
        dstImg(x, y) = newVal * p_maxRange;
        return dstImg;
    }

    // --- Path B. Color Image ---
    else {
        // 1. Extract B, G, R Value
        Halide::Expr B = srcImg(x, y, 0) * invMaxRange;
        Halide::Expr G = srcImg(x, y, 1) * invMaxRange;
        Halide::Expr R = srcImg(x, y, 2) * invMaxRange;

        // 2. Convert to HSL
        std::vector<Halide::Expr> hslImg = HalideColorSpace::BGR2HSL(B, G, R);

        // 3. Extract current Luminance Value
        Halide::Expr currL = hslImg[2];

        // 4. Calculate the weight
        Halide::Expr weight =
            HalideImageUtils::calculateBrightWeight(currL, p_underVal, p_upperVal);

        // 5. Calculate the delta Luminance
        Halide::Expr deltaL = weight * p_highlightFactor;

        // 6. Calculate the new Luminance and clamp it
        Halide::Expr newL = Halide::clamp(deltaL + currL, 0.0f, 1.0f);

        // 7. Convert back to BGR
        Halide::Expr H = hslImg[0];
        Halide::Expr S = hslImg[1];
        std::vector<Halide::Expr> bgrImg = HalideColorSpace::HSL2BGR(H, S, newL);

        // 8. Channel Selection
        Halide::Expr val =
            Halide::select(c == 0, bgrImg[0], Halide::select(c == 1, bgrImg[1], bgrImg[2]));

        // 7. Assign the new Luminance to Destination Image and denormalize to original Bit Depth
        Halide::Func dstImg("adjust_highlight_color_image");
        dstImg(x, y, c) = val * p_maxRange;
        return dstImg;
    }
}

cv::Mat AdjustHighlight::apply(const cv::Mat& srcImg) {
    // 1. Calculate the change factor
    float highlightFactor = m_highlight / HIGHLIGHT_SCALING_FACTOR;

    // --- Path A. Color image ---
    if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        // 1. Convert to HSL
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        // 2. Find the min & max value on the 512x512 thumbnail to calculate the weight later
        cv::Mat thumbnail = ImageUtils::createThumbnail(hslImg);
        auto minMaxVal = ImageUtils::calculateMinMax(thumbnail, 2);
        float minL = std::get<0>(minMaxVal);
        float maxL = std::get<1>(minMaxVal);

        // 3. Calculate the weight
        auto weightParams = ImageUtils::precalculateWhiteWeightParams(
            minL, maxL, WEIGHT_RANGE_LOWER, WEIGHT_RANGE_UPPER);

        int len = hslImg.cols * 3;  // 1D Array

        // clang-format off
        // 4. Iteration through the whole image
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < srcImg.rows; y++) {
            // 4.1 Get the pointer of the first pixel each line on the hsl image
            float* __restrict hslPtr = hslImg.ptr<float>(y);

            // 4.2 Calculate pixel-wise
            for (int x = 2; x < len; x += 3) {
                // 4.2.1 Extract the current Luminance Value
                float currL = hslPtr[x];

                // 4.2.2 Calculate the weight
                float weight = ImageUtils::calculateBrightWeight(currL, weightParams);

                // 4.2.3 Calculate the delta Luminance
                float deltaL = weight * highlightFactor;

                // 4.2.4 Calculate the new Luminance and clamp it
                float newL = std::clamp(currL + deltaL, 0.0f, 1.0f);

                // 4.2.5 Assign the new Luminance to the Destination Image
                hslPtr[x] = newL;
            }
        }

        // 5. Convert back to BGR with original Bit Depth
        if (srcImg.depth() == CV_8U) {
            return ColorSpace::convertHSL2BGR(hslImg, 8);
        } else {
            return ColorSpace::convertHSL2BGR(hslImg, 16);
        }
    }

    // --- Path B. 8 bit Gray Image ---
    else if (srcImg.type() == CV_8UC1) {
        return highlightGrayImgTemplate<uint8_t>(srcImg, highlightFactor);
    }

    // --- Path C. 16 bit Gray Image
    else if (srcImg.type() == CV_16UC1) {
        return highlightGrayImgTemplate<uint16_t>(srcImg, highlightFactor);
    }

    else {
        std::cerr << "Error: unsupported image type\n";
        return cv::Mat();
    }
}