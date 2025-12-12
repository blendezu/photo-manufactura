#pragma once
#include <Halide.h>

#include <algorithm>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/saturate.hpp>
#include <string>

#include "operation_base.h"

/**
 * @class TintMagenta
 * @brief Applies a subtle magenta tint to color images.
 *
 * This class inherits from ImageOperation and uses a template-based approach
 * to efficiently process both 8-bit and 16-bit 3-channel images.
 *
 * The intensity of the magenta tint can be adjusted with the 'tint' parameter,
 * which is clamped within the range [-100, 100] --> [tint, magenta]
 */
class TintMagenta : public HalideOperation {
   private:
    int m_tint;

    // --- Constant Parameters ---
    static constexpr float TINT_SCALING_FACTOR = 200.0f;

    // --- Halide Runtime Parameters ---
    Halide::Param<float> p_timaFactor{"timaFactor"};

   public:
    TintMagenta(int value) : m_tint(value) {
        p_timaFactor.set(0.0f);
    }

    void prepareParameters(const cv::Mat& srcImg) override;

    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    /**
     * @brief Applies magenta tint to the provided image.
     * @param srcImg Input image (CV_8UC3 or CV_16UC3)
     * @return New image with modified green channel
     */
    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Tint Magenta";
    }

    std::string getSettings() const override {
        return "tint: " + std::to_string(m_tint);
    }

    void setTint(int value) {
        m_tint = std::clamp(value, -100, 100);
    }

    int getTint() {
        return m_tint;
    }

   private:
    template <typename V, typename T>
    cv::Mat tintMagentaTemplate(const cv::Mat& srcImg, float changeFactor) {
        // 1. Create the destination Image
        cv::Mat dstImg(srcImg.size(), srcImg.type());

        // clang-format off
        // 2. Iteration through the Image using OpenMP for Parallelism
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < srcImg.rows; y++) {
            // 2.1 Get the pointer of the first pixel each lines
            const V* __restrict srcPtr = srcImg.ptr<V>(y);
            V* __restrict dstPtr = dstImg.ptr<V>(y);

            // 2.2 Calculate pixel-wise
            for (int x = 0; x < srcImg.cols; x++) {
                // 2.2.1 Extract the BGR Values
                T B = srcPtr[x][0];
                T G = srcPtr[x][1];
                T R = srcPtr[x][2];

                // 2.2.2 Calculate only new Green Value
                T newG = cv::saturate_cast<T>(G * changeFactor);

                // 2.2.3 Assign the new Values to Destination Image
                dstPtr[x] = V(B, newG, R);
            }
        }
        return dstImg;
    }
};