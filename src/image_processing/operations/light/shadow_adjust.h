#pragma once
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <string>

#include "Halide.h"
#include "image_utils.h"
#include "operation_base.h"

/**
 * @brief Adjusts the shadow of an image by modifying luminance in dark area
 * * This class inherits from HalideOperation to utilize GPU acceleration.
 * On GPU mode, it calculates image statistics (min/max luminance) on the CPU and
 * performs the pixel-wise adjustment using a Halide JIT graph on GPU.
 */
class AdjustShadow : public HalideOperation {
   private:
    int m_shadow; /**< Adjustment strength: Range [-100, 100] */

    // --- Halide Runtime Paramter ---
    // These allow modifying values without recompiling the JIT graph.

    Halide::Param<float> p_shadowFactor{"p_shadow_factor"};
    Halide::Param<float> p_underVal{"p_shadow_under"};
    Halide::Param<float> p_upperVal{"p_shadow_upper"};
    Halide::Param<float> p_maxRange{"p_shadow_maxrange"};

    // --- Constants for Weight Calculation ---
    static constexpr float SHADOW_SCALING_FACTOR = 800.0f;
    static constexpr float WEIGHT_RANGE_LOWER = 0.3f;
    static constexpr float WEIGHT_RANGE_UPPER = 0.6f;

   public:
    /**
     * @brief Construct a new Adjust Shadow object
     * @param value Initial strength value (-100 to 100)
     */
    AdjustShadow(int value) : m_shadow(value) {
        p_shadowFactor.set(0.0f);
        p_underVal.set(0.0f);
        p_upperVal.set(1.0f);
        p_maxRange.set(255.0f);
    }

    bool supportsHalide() const override {
        return true;
    }

    bool requiresFreshStats() const override {
        return true;
    }

    /**
     * @brief Calculate min/max statistics on the CPU
     *
     * @param srcImg
     */
    void prepareParameters(const cv::Mat& srcImg) override;

    /**
     * @brief Apply the shadow adjustment using CPU
     *
     * @param srcImg
     * @return cv::Mat
     */
    cv::Mat apply(const cv::Mat& srcImg) override;

    /**
     * @brief Build the Halide graph
     *
     * @param srcImg
     * @param x
     * @param y
     * @param c
     * @return Halide::Func
     */
    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    // --- Getters / Setters / Metadata ---
    std::string getName() const override {
        return "Shadow";
    }

    std::string getSettings() const override {
        return "Shadow: " + std::to_string(m_shadow);
    }

    void setShadow(int value) {
        m_shadow = value;
    }

    int getShadow() {
        return m_shadow;
    }

   private:
    /**
     * @brief Template for Adjust shadow on Gray image
     * T: 8-bit: uint8_t, 16-bit: uint16_t
     * @param srcImg source image
     * @param shadowFactor change factor
     */
    template <typename T>
    cv::Mat shadowGrayImgTemplate(const cv::Mat& srcImg, float shadowFactor) {
        // 1. Check if the image is empty
        if (srcImg.empty()) {
            std::cerr << "Error: empty input image\n";
            return cv::Mat();
        }

        // 2. Calculate maxRange and invMaxRange
        float invMaxRange = 0.0f;  // --> to avoid division in the for loop
        float maxRange = 0.0f;
        if (srcImg.type() == CV_8UC1) {
            invMaxRange = 1 / 255.0f;
            maxRange = 255.0f;
        } else if (srcImg.type() == CV_16UC1) {
            invMaxRange = 1 / 65535.0f;
            maxRange = 65535.0f;
        }

        // 3. Output image
        cv::Mat dstImg(srcImg.size(), srcImg.type());

        // 4. Calculate min/max of the image to calculate weight parameters
        cv::Mat thumbnail =
            ImageUtils::createThumbnail(srcImg);  // --> thumbnail image for better performance
        auto minMaxVal = ImageUtils::calculateMinMax(thumbnail, 0);
        float minVal = std::get<0>(minMaxVal);
        float maxVal = std::get<1>(minMaxVal);

        // 5. Calculate weight parameters
        auto weightParams = ImageUtils::precalculateDarkWeightParams(
            minVal, maxVal, WEIGHT_RANGE_LOWER, WEIGHT_RANGE_UPPER);

        // clang-format off
        // 6. Parallelize over rows
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < srcImg.rows; y++) {
            // 6.1. Get pointers to the current row
            // __restrict: tell the compiler that the pointer is not aliased
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

            // 6.2. Iterate over pixels in the row
            for (int x = 0; x < srcImg.cols; x++) {
                // 6.2.1. Get the current pixel value
                float currVal = srcPtr[x] * invMaxRange;  // 0 -> 1

                // 6.2.2. Calculate the weight
                float weight = ImageUtils::calculateDarkWeight(currVal, weightParams);

                // 6.2.3. Calculate the brightness change
                float deltaVal = weight * shadowFactor;

                // 6.2.4. Calculate the new pixel value
                float newVal = std::clamp(currVal + deltaVal, 0.0f, 1.0f);

                // 6.2.5. Convert back to original bit depth
                dstPtr[x] = static_cast<T>(newVal * maxRange);
            }
        }

        // 7. Return the processed image
        return dstImg;
    }
};