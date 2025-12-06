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

    // Cache for Statistic
    float m_minL;
    float m_maxL;

    // --- Constants for Weight Calculation ---

    // The slider value is devided by this factor to get usable float multiplier
    static constexpr float WHITE_SCALING_FACTOR = 800.0f;

    // Define the range of luminance affected by the white adjustment.
    static constexpr float WEIGHT_RANGE_LOWER =
        0.1f; /**< 0.7 means the effect starts at 70% brightness */
    static constexpr float WEIGHT_RANGE_UPPER =
        0.2f; /**< 0.9 means the effect starts at 90% brightness */

   public:
    /**
     * @brief Construct a new Adjust White operation.
     * @param value Initial strength value (-100 to 100).
     */
    AdjustWhite(int value) : m_white(value), m_minL(0.0f), m_maxL(1.0f) {
        p_underVal.set(0.0f);
        p_upperVal.set(1.0f);
        p_whiteFactor.set(0.0f);
        p_maxRange.set(255.0f);
    }

    bool supportsHalide() const override {
        return true;
    }

    bool requiresFreshStats() const override {
        return true;
    }

    // calculate Parameters on CPU
    void prepareParameters(const cv::Mat& srcImg) override;

    /**
     * @brief Prepares parameters and executes the pipeline.
     * * Caculates global min/max statistics using OpenCV (CPU)
     * and delegates the pixel processing to the Halide backend (GPU/CPU).
     * @param srcImg Input image (CV_8U or CV_16_U, 1 or 3 channels).
     * @return cv::Mat processed image
     */
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

        // 3. Statistics Calculation min/max on a thumbnail image for better performance
        cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);
        auto minMaxVal = ImageUtils::calculateMinMax(thumbnail, 0);
        float minVal = std::get<0>(minMaxVal);
        float maxVal = std::get<1>(minMaxVal);

        // Calculate Parameters for Weight Calculation later
        auto weightParams = ImageUtils::precalculateWhiteWeightParams(
            minVal, maxVal, WEIGHT_RANGE_LOWER, WEIGHT_RANGE_UPPER);

        // clang-format off
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < srcImg.rows; y++) {
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                float normedVal = srcPtr[x] / maxRange;

                float weight = ImageUtils::calculateBrightWeight(normedVal, weightParams);

                float deltaVal = weight * whiteFactor;
                float newVal = std::clamp(normedVal + deltaVal, 0.0f, 1.0f);

                // Convert back to original Bit Depth
                dstPtr[x] = static_cast<T>(newVal * maxRange);
            }
        }
        return dstImg;
    }
};