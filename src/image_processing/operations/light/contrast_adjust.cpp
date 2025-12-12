#include "contrast_adjust.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <cstdint>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/saturate.hpp>
#include <vector>

#include "color_space.h"
#include "halide_color_space.h"
#include "image_algorithms.h"

// --- Determine the max Range of the Source Image ---
void AdjustContrast::prepareParameters(const cv::Mat& srcImg) {
    // 1. Determine maxRange
    if (srcImg.depth() == CV_8U) {
        p_maxRange.set(255.0f);
    }

    else if (srcImg.depth() == CV_16U) {
        p_maxRange.set(65535.0f);
    } else {
        std::cerr << "[AdjustContrast] ❌ Error: Unsupported Image Type\n";
        return;
    }

    // 2. Calculate and update the change factor
    float contrastFactor = 1.0f + m_contrast / CONTRAST_SCALING_FACTOR;
    p_contrastFactor.set(contrastFactor);
}

// ------ GPU --------
Halide::Func AdjustContrast::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                                        Halide::Var c) {
    // --- Path A. Gray Image ---
    if (srcImg.dimensions() == 2) {
        // 1. Calculate half value for calculation new Value
        Halide::Expr halfMaxRange = p_maxRange / 2.0f;

        // 2. Calculate new Value
        Halide::Expr newVal = (srcImg(x, y) - halfMaxRange) * p_contrastFactor + halfMaxRange;

        // 3. Assign new Value to Destination Image
        Halide::Func dstImg("contrast_adjust_gray_image");
        dstImg(x, y) = newVal;
        return dstImg;
    }

    // --- Path B. Color Image ---

    // 1. Extract BGR and inverse it to convert to HSL
    // Inverse maxRange to avoid Division
    Halide::Expr invMaxRange = 1.0f / p_maxRange;
    Halide::Expr B = srcImg(x, y, 0) * invMaxRange;
    Halide::Expr G = srcImg(x, y, 1) * invMaxRange;
    Halide::Expr R = srcImg(x, y, 2) * invMaxRange;

    // 2. Convert to HSL
    std::vector<Halide::Expr> hslImg = HalideColorSpace::BGR2HSL(B, G, R);

    // 3. Extract HSL
    Halide::Expr H = hslImg[0];
    Halide::Expr S = hslImg[1];
    Halide::Expr currL = hslImg[2];

    // 5. Calculate new Luminance Value
    Halide::Expr newL = ImageAlgorithms::apply_contrast(currL, p_contrastFactor);

    // 6. Convert back to BGR
    std::vector<Halide::Expr> bgrImg = HalideColorSpace::HSL2BGR(H, S, newL);

    // 7. Selection Channel
    Halide::Expr val =
        Halide::select(c == 0, bgrImg[0], Halide::select(c == 1, bgrImg[1], bgrImg[2]));

    // 8. Assign and denormalize the new Value to Destination Image
    Halide::Func dstImg("contrast_adjust_image");
    dstImg(x, y, c) = val * p_maxRange;
    return dstImg;
}

// ------ CPU --------
cv::Mat AdjustContrast::apply(const cv::Mat& srcImg) {
    float contrastFactor = 1.0f + m_contrast / CONTRAST_SCALING_FACTOR;

    // --- Path A. 8-bit Gray Image ---
    if (srcImg.type() == CV_8UC1) {
        return contrastGrayImgTemplate<uint8_t>(srcImg, contrastFactor);

    }

    // --- Path B. 16-bit Gray Image ---
    else if (srcImg.type() == CV_16UC1) {
        return contrastGrayImgTemplate<uint16_t>(srcImg, contrastFactor);
    }

    // --- Path C. Color Image ---
    else if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        // 1. Convert to HSL Image
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        // 2. Calculate the length 1D Array
        int len = hslImg.cols * 3;

        // clang-format off
        // 3. Iteration through the Image
        #pragma omp parallel for
        // clang-format on

        for (int y = 0; y < hslImg.rows; y++) {
            // 3.1 Get the pointer of first pixel each line
            float* __restrict hslPtr = hslImg.ptr<float>(y);

            // 3.2 Calculate pixel-wise
            // Using only Luminance chanel, which in the Positions 2, 5, 8 , ...
            for (int x = 2; x < len; x += 3) {
                // 3.2.1 Extract the current Luminance
                float currL = hslPtr[x];

                // 3.2.2 Calculate the new Luminance
                float newL = (currL - 0.5f) * contrastFactor + 0.5f;

                // 3.2.3 Clamp the new Luminance
                newL = std::clamp(newL, 0.0f, 1.0f);

                // 3.2.4 Assign the new Luminance to the Destination Image
                hslPtr[x] = newL;
            }
        }
        // 4. Convert back to BGR Image with original Bit Depth
        if (srcImg.type() == CV_16UC3) {
            return ColorSpace::convertHSL2BGR(hslImg, 16);
        } else {
            return ColorSpace::convertHSL2BGR(hslImg, 8);
        }
    }

    // --- Path D. Unsupported Image Type ---
    else {
        std::cerr << "Error: unsupported image type";
        return cv::Mat();
    }
}