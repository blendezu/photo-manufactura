#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <string>

#include "image_utils.h"
#include "operation_base.h"

/**
 * @brief Adjusts the highlight of an image by modifying luminance in bright area
 * * This class inherits from HalideOperation to utilize GPU acceleration.
 * On GPU mode it calculates image statistics (min/max luminance) on the CPU and performs the
 * pixel-wise adjustment using a Halide JIT graph.
 */
class AdjustHighlight : public HalideOperation {
   private:
    int m_highlight; /**< Adjustment strength: Range [-100, 100] */

    // --- Halide Runtime Paramter ---
    // These allow modifying values without recompiling the JIT graph.

    Halide::Param<float> p_underVal{"p_highlight_under"};
    Halide::Param<float> p_upperVal{"p_highlight_upper"};
    Halide::Param<float> p_highlightFactor{"p_highlight_factor"};
    Halide::Param<float> p_maxRange{"p_max_range"};

    // --- Constants for Weight Calculation ---

    // The slider value is divided by this factor to get usable float multiplier
    static constexpr float HIGHLIGHT_SCALING_FACTOR = 800.0f;

    // Define the range of luminance affected by the white adjustment.
    // This means values above 70% have the weight is 1, and reduce to 0
    static constexpr float WEIGHT_RANGE_LOWER = 0.4f;
    static constexpr float WEIGHT_RANGE_UPPER = 0.7f;

   public:
    /**
     * @brief Construct a new Adjust Highlight operation.
     * @param value Initial strength value [-100, 100].
     */
    AdjustHighlight(int value) : m_highlight(value) {
        p_underVal.set(0.0f);
        p_upperVal.set(1.0f);
        p_highlightFactor.set(0.0f);
        p_maxRange.set(255.0f);
    }

    /**
     * @brief Decide if fresh statistics calculations are required
     * @return true
     */
    bool requiresFreshStats() const override {
        return false;
    }

    /**
     *@brief Calculate Statistics on CPU before Processing on GPU
     */
    void prepareParameters(const cv::Mat& srcImg) override;

    /**
     * @brief Define the Halide computation graph
     */

    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    /**
    * @brief Apply operation on CPU
    @ @param srcImg Source image.
     */
    cv::Mat apply(const cv::Mat& srcImg) override;

    // --- Getter / Setters / Metadata
    std::string getName() const override {
        return "Highlight";
    }

    std::string getSettings() const override {
        return "highlight: " + std::to_string(m_highlight);
    }

    void setHighlight(int value) {
        m_highlight = std::clamp(value, -100, 100);
    }

    int getHighlight() {
        return m_highlight;
    }

   private:
    template <typename T>
    cv::Mat highlightGrayImgTemplate(const cv::Mat srcImg, float changeFactor) {
        // 1. Check if the input image is empty
        if (srcImg.empty()) {
            std::cerr << "Error in AdjustHighlight: empty input image\n";
            return cv::Mat();
        }

        // 2. Check if the image is a 8 or 16 bit Gray Image
        if (srcImg.type() != CV_8UC1 && srcImg.type() != CV_16UC1) {
            std::cerr << "Error in AdjustHighlight: expect only gray image\n";
        }

        // 3. Calculate parameters
        cv::Mat dstImg(srcImg.size(), srcImg.type());  // output image
        float maxRange = 0.0f;
        float invMaxRange = 0.0f;  // --> to avoid division in the for loop
        if (srcImg.depth() == CV_8U) {
            maxRange = 255.0f;
            invMaxRange = 1 / maxRange;
        } else {
            maxRange = 65535.0f;
            invMaxRange = 1 / maxRange;
        }

        // 4. Find min max value to calculate weight on the thumbnail
        cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);
        auto minMaxVal = ImageUtils::calculateMinMax(thumbnail, 0);
        float minL = std::get<0>(minMaxVal);
        float maxL = std::get<1>(minMaxVal);

        auto weightParams = ImageUtils::precalculateWhiteWeightParams(
            minL, maxL, WEIGHT_RANGE_LOWER, WEIGHT_RANGE_UPPER);

        // clang-format off
        // 5. Iteration through the whole image
        #pragma omp parallel for
        // clang-format on

        for (int y = 0; y < srcImg.rows; y++) {
            // 5.1 Get the pointers of source and destination image
            // __restrict: tell the compiler that the pointer is not aliased
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

            // 5.2 Calculate pixel-wise
            for (int x = 0; x < srcImg.cols; x++) {
                // 5.2.1 Calculate the normed current Value
                float currVal = srcPtr[x] * invMaxRange;

                // 5.2.2 Calculate the weight
                float weight = ImageUtils::calculateBrightWeight(currVal, weightParams);

                // 5.2.3 Calculate the delta Value
                float deltaL = weight * changeFactor;

                // 5.2.4 Calculate the new Value and clamp it
                float newL = std::clamp(currVal + deltaL, 0.0f, 1.0f);

                // 5.2.5 Assign it to the Destination Image
                dstPtr[x] = static_cast<T>(newL * maxRange);
            }
        }
        return dstImg;
    }
};