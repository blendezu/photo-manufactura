#include "saturation_adjust.h"

#include <Halide.h>

#include <algorithm>
#include <iostream>
#include <opencv2/core/mat.hpp>
#include <vector>

#include "color_space.h"
#include "halide_color_space.h"
#include "image_algorithms.h"

// --- Calculation Halide Runtime Parameters ---
void AdjustSaturation::prepareParameters(const cv::Mat& srcImg) {
    // --- Path A. Gray Image ---
    if (srcImg.channels() == 1) {
        std::cerr << "[AdjustSaturation] ⚠️ Warning: Support only Color Image\n";
        return;
    }

    // --- Path B. Color Image ---
    // 1. Calculation parameters
    float saturationFactor = 1.0f + m_saturation / SATURATION_SCALING_FACTOR;
    float maxRange = (srcImg.depth() == CV_8U) ? 255.0f : 65535.0f;

    // 2. Assign it
    p_saturationFactor.set(saturationFactor);
    p_maxRange.set(maxRange);
}

Halide::Func AdjustSaturation::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                                          Halide::Var c) {
    if (srcImg.dimensions() == 3) {
        // 1. Inverse maxRange to avoid Divison
        Halide::Expr invMaxRange = Halide::select(p_maxRange < 10e-6f, 0.0f, 1.0f / p_maxRange);

        // 2. Extract BGR Values and normalize them
        Halide::Expr B = srcImg(x, y, 0) * invMaxRange;
        Halide::Expr G = srcImg(x, y, 1) * invMaxRange;
        Halide::Expr R = srcImg(x, y, 2) * invMaxRange;

        // 3. Convert to HSL
        std::vector<Halide::Expr> hslImg = HalideColorSpace::BGR2HSL(B, G, R);

        // 4. Extract the HSL Values
        Halide::Expr H = hslImg[0];
        Halide::Expr currS = hslImg[1];
        Halide::Expr L = hslImg[2];

        // 5. Calculate delta Saturation Value and clamp it
        Halide::Expr newS = ImageAlgorithms::apply_saturation(currS, p_saturationFactor);

        // 6. Convert back to BGR
        std::vector<Halide::Expr> bgrImg = HalideColorSpace::HSL2BGR(H, newS, L);

        // 7. Channel Setection
        Halide::Expr val =
            Halide::select(c == 0, bgrImg[0], Halide::select(c == 1, bgrImg[1], bgrImg[2]));

        // 8. Denormalize the new BGR values and assign it to Destination Image
        Halide::Func dstImg("adjust_saturation_color_image");
        dstImg(x, y, c) = val * p_maxRange;

        return dstImg;
    }

    else {
        return srcImg;
    }
}

cv::Mat AdjustSaturation::apply(const cv::Mat& srcImg) {
    // 1. Check if the input image is empty
    if (srcImg.empty()) {
        std::cerr << "Error in AdjustSaturation: empty image\n";
        return cv::Mat();
    }

    // 2. Support only Color Image, if Gray Image return it.
    if (srcImg.channels() == 1) {
        return srcImg;
    }

    // 3. Convert to HSL Image
    cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);
    float satFactor = 1.0f + m_saturation / SATURATION_SCALING_FACTOR;

    // 4. Length of 1D Array
    int len = hslImg.cols * 3;

    // clang-format off
    // 5. Iteration through the HSL Image
    #pragma omp parallel for
    // clang-format on
    for (int y = 0; y < hslImg.rows; y++) {
        // 5.1 Get the pointer of the first pixel of the line
        float* __restrict hslPtr = hslImg.ptr<float>(y);

        // 5.1 Execute pixel-wise
        // Saturation channel is at 1, 4, 7, ...
        for (int x = 1; x < len; x += 3) {
            // 5.1.1 Extract the current Saturation Value
            float currS = hslPtr[x];

            // 5.1.2 Calculate the new Satureation Value
            float newS = currS * satFactor;

            // 5.1.3 Clamp it
            newS = std::clamp(newS, 0.0f, 1.0f);

            // 5.1.4 Assign the new Values to the HSL Image
            hslPtr[x] = newS;
        }
    }

    // 6. Convert back to BGR Image with 0riginal Bit Depth
    if (srcImg.depth() == CV_8U) {
        return ColorSpace::convertHSL2BGR(hslImg, 8);

    } else {
        return ColorSpace::convertHSL2BGR(hslImg, 16);
    }
}