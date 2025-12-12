#include "vibrance_adjust.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <vector>

#include "color_space.h"
#include "halide_color_space.h"
#include "halide_image_utils.h"

// --- Calculate Halide Runtime Parameters ---
void AdjustVibrance::prepareParameters(const cv::Mat& srcImg) {
    // --- Path A. Gray Image ---
    if (srcImg.channels() == 1) {
        std::cerr << "[AdjustVibrance] ⚠️ Warning: Doesn't support Gray Image\n";
        return;
    }

    // --- Path B. Color Image ---
    float maxRange = (srcImg.depth() == CV_8U) ? 255.0f : 65535.0f;
    float vibranceFactor = m_vibrance / VIBRANCE_SCALING_FACTOR;
    p_maxRange.set(maxRange);
    p_vibranceFactor.set(vibranceFactor);
    p_lowerThreshold.set(LOWER_THRESHOLD);
    p_upperThreshold.set(UPPER_THRESHOLD);
}

Halide::Func AdjustVibrance::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                                        Halide::Var c) {
    // --- Path A. Gray Image ---
    if (srcImg.dimensions() == 2) {
        return srcImg;
    }

    // --- Path B. Color Image ---
    // 1. Extract BGR Values and normalize them
    Halide::Expr invMaxRange = 1.0f / p_maxRange;  // to avoid Division
    Halide::Expr B = srcImg(x, y, 0) * invMaxRange;
    Halide::Expr G = srcImg(x, y, 1) * invMaxRange;
    Halide::Expr R = srcImg(x, y, 2) * invMaxRange;

    // 2. Convert to HSL
    std::vector<Halide::Expr> hslImg = HalideColorSpace::BGR2HSL(B, G, R);

    // 3. Extract the HSL Values
    Halide::Expr H = hslImg[0];
    Halide::Expr currS = hslImg[1];
    Halide::Expr L = hslImg[2];

    // 4. Calculate the weight using the calculateDarkWeight from HalideImageUtils
    Halide::Expr weight =
        HalideImageUtils::calculateDarkWeight(currS, p_lowerThreshold, p_upperThreshold);

    // 5. Calculate new Saturation Value and clamp it
    Halide::Expr newS = Halide::clamp(currS + weight * p_vibranceFactor, 0.0f, 1.0f);

    // 6. Convert back to BGR
    std::vector<Halide::Expr> bgrImg = HalideColorSpace::HSL2BGR(H, newS, L);

    // 7. Channel Selection
    Halide::Expr val =
        Halide::select(c == 0, bgrImg[0], Halide::select(c == 1, bgrImg[1], bgrImg[2]));

    // 8. Denormalize BGR Values and assign to the Destination Image
    Halide::Func dstImg("vibrance_adjust_color_image");
    dstImg(x, y, c) = val * p_maxRange;

    return dstImg;
}

cv::Mat AdjustVibrance::apply(const cv::Mat& srcImg) {
    // Check if the Source Image is empty
    if (srcImg.empty()) {
        std::cerr << "Error in AdjustVibrance: empty input image\n";
        return cv::Mat();
    }

    // --- Path A. Gray Image ---
    if (srcImg.channels() == 1) {
        std::cerr << "[AdjustVibrance] ⚠️ Warning: Doesn't support Gray Image\n";
        return srcImg;
    }

    // --- Path B. Color Image ----
    // 1. Calculate the vibranceFactor
    float vibranceFactor = m_vibrance / VIBRANCE_SCALING_FACTOR;

    // 2. Convert Source Image to HSL
    cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

    // 3. Calculate the length of 1D Array
    int len = hslImg.cols * 3;

    // clang-format off
    // 4. Iteration through the Image using OpenMP for parallelism
    #pragma omp parallel for
    // clang-format on
    for (int y = 0; y < hslImg.rows; y++) {
        // 4.1 Get the pointer of the first pixel each line
        float* __restrict hslPtr = hslImg.ptr<float>(y);

        // 4.2 Calculate pixel-wise
        // Saturation Channel is at 1, 4, 7, ...
        for (int x = 1; x < len; x += 3) {
            // 4.2.1 Extract the current Saturation Value
            float currS = hslPtr[x];

            // 4.2.2 Calculate the weight
            float weight = caculateWeight(currS);

            // 4.2.3 Calculate the new Saturation Value and clamp it
            float newS = currS + weight * vibranceFactor;
            newS = std::clamp(newS, 0.0f, 1.0f);

            // 4.2.4 Assign the new Saturation Value back
            hslPtr[x] = newS;
        }
    }

    // 5. Convert back to BGR with original Bit Depth
    if (srcImg.depth() == CV_8U) {
        return ColorSpace::convertHSL2BGR(hslImg, 8);
    } else {
        return ColorSpace::convertHSL2BGR(hslImg, 16);
    }
}