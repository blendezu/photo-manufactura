#pragma once

#include <opencv2/core/hal/interface.h>

#include <opencv2/core/mat.hpp>
#include <string>

#include "Halide.h"
#include "image_utils.h"
#include "operation_base.h"
/**
 * @brief Adjusts the white point of an image by modifying luminance in bright area
 * * This class inherits from HalideOperation to utilize GPU acceleration.
 * It calculates image statistics (min/max luminance) on the CPU and
 * performs the pixel-wise adjustment using a Halide JIT graph.
 * The effect is strong in the brightness area, reducing in the mid and zero in the dark area.
 */
class AdjustWhite : public HalideOperation {
   private:
    int m_white;  //**< Adjustment strength: Range [-100, 100] */

    // --- Halide Runtime Parameter ---
    // These allow modifying values without recompiling the JIT graph.

    Halide::Param<float> p_underVal{"p_white_under"};
    Halide::Param<float> p_upperVal{"p_white_upper"};
    Halide::Param<float> p_whiteFactor{"p_white_factor"};
    Halide::Param<float> p_maxRange{"p_white_maxrange"};

    // --- Constants for Weight Calculation ---

    // The slider value is devided by this factor to get usable float multiplier
    static constexpr float WHITE_SCALING_FACTOR = 800.0f;

    // Define the range of luminance affected by the white adjustment.
    static constexpr float WEIGHT_RANGE_LOWER = 0.7f;
    static constexpr float WEIGHT_RANGE_UPPER = 0.9f;

   public:
    /**
     * @brief Construct a new Adjust White operation.
     * @param value Initial strength value (-100 to 100).
     */
    AdjustWhite(int value) : m_white(value) {
        p_underVal.set(0.0f);
        p_upperVal.set(1.0f);
        p_whiteFactor.set(0.0f);
        p_maxRange.set(255.0f);
    }

    bool requiresFreshStats() const override {
        return false;
    }

    /**
     * @brief Prepares parameters and executes the pipeline.
     * * Caculates global min/max statistics using OpenCV (CPU)
     * and delegates the pixel processing to the Halide backend (GPU/CPU).
     * @param srcImg Input image (CV_8U or CV_16_U, 1 or 3 channels).
     */
    void prepareParameters(const cv::Mat& srcImg) override;

    cv::Mat apply(const cv::Mat& srcImg) override;

    /**
     * @brief Defines the Halide computation graph
     */
    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    // --- Getters / Settets / Metadata ---

    std::string getName() const override {
        return "White";
    }

    std::string getSettings() const override {
        return "white: " + std::to_string(m_white);
    }

    void setWhite(int value) {
        m_white = std::clamp(value, -100, 100);
    }

    int getWhite() {
        return m_white;
    }

   private:
    /**
     * @brief Template for Adjust white on Gray image
     * T: 8-bit: uint8_t, 16-bit: uint16_t
     * @param srcImg source image
     * @param whiteFactor change factor
     */
    template <typename T>
    cv::Mat whiteGrayImgTemplate(const cv::Mat& srcImg, float whiteFactor) {
        // 1. Output image
        cv::Mat dstImg(srcImg.size(), srcImg.type());

        // 2. Find the Max Range Value based on Bit Depth
        float maxRange = 0.0f;
        if (srcImg.depth() == CV_8U) {
            maxRange = 255.0f;
        } else {
            maxRange = 65535.0f;
        }
        float invMaxRange = 1.0 / maxRange;  // to avoid Division later

        // 3. Statistics Calculation min/max on a thumbnail image for better performance
        cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);
        auto minMaxVal = ImageUtils::calculateMinMax(thumbnail, 0);
        float minVal = std::get<0>(minMaxVal);
        float maxVal = std::get<1>(minMaxVal);

        // 4. Calculate Parameters for Weight Calculation later
        auto weightParams = ImageUtils::precalculateWhiteWeightParams(
            minVal, maxVal, WEIGHT_RANGE_LOWER, WEIGHT_RANGE_UPPER);

        // clang-format off
        // 5. Parallelize over rows using OpenMP
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < srcImg.rows; y++) {
            // 5.1 Get the pointers of source and destination image
            // Using __restrict to tell the compiler that the pointer are not aliased.
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

            // 5.2 Calculate pixel-wise
            for (int x = 0; x < srcImg.cols; x++) {
                // 5.2.1 Get and normalized current Value
                float currVal = srcPtr[x] * invMaxRange;

                // 5.2.2 Calculate the weight
                float weight = ImageUtils::calculateBrightWeight(currVal, weightParams);

                // 5.3.3 Calculate the delta Value
                float deltaVal = weight * whiteFactor;

                // 5.3.4 Calculate the new Value and clamp it
                float newVal = std::clamp(currVal + deltaVal, 0.0f, 1.0f);

                // Convert back to original Bit Depth
                dstPtr[x] = static_cast<T>(newVal * maxRange);
            }
        }
        return dstImg;
    }
};