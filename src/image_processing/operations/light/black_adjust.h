#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <string>

#include "image_utils.h"
#include "operation_base.h"

class AdjustBlack : public HalideOperation {
   private:
    int m_black;

    // --- Constant Parameters ---
   public:
    static constexpr float BLACK_SCALING_FACTOR = 800.0f;
    static constexpr float LOWER_THRESHOLD_PERCENT = 0.1f;
    static constexpr float UPPER_THRESHOLD_PERCENT = 0.3f;

   private:
    // --- Halide Runtime Parameters ---
    Halide::Param<float> p_maxRange{"p_black_maxRange"};
    Halide::Param<float> p_blackFactor{"blackFactor"};
    Halide::Param<float> p_lowerPoint{"black_lowerPoint"};
    Halide::Param<float> p_upperPoint{"black_upperPoint"};

   public:
    AdjustBlack(int value) : m_black(value) {
        p_maxRange.set(255.0f);
        p_blackFactor.set(0.0f);
        p_lowerPoint.set(0.0f);
        p_upperPoint.set(1.0f);
    }

    bool requiresFreshStats() const override {
        return false;
    }

    void prepareParameters(const cv::Mat& srcImg) override;

    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Black";
    }

    std::string getSettings() const override {
        return "black: " + std::to_string(m_black);
    }

    void setBlack(int value) {
        m_black = std::clamp(value, -100, 100);
    }

    int getBlack() {
        return m_black;
    }

   private:
    template <typename T>
    cv::Mat blackGrayImgTemplate(const cv::Mat& srcImg, float blackFactor) {
        // 1. Create Destination Image
        cv::Mat dstImg(srcImg.size(), srcImg.type());

        // 2. Determine the max Range based on Bit Depth
        float maxRange = 0.0f;
        (srcImg.depth() == CV_8U) ? maxRange = 255.0f : maxRange = 65535.0f;  // 8 bit or 16 bit
        const float invMaxRange = 1.0f / maxRange;  // to avoid division in the for loop

        // 3. Find min/max on the 512x512 thumbnail to improve performance
        cv::Mat thumbnail = ImageUtils::createThumbnail(srcImg);
        auto minMaxResult = ImageUtils::calculateMinMax(thumbnail, 0);
        float minL = std::get<0>(minMaxResult);
        float maxL = std::get<1>(minMaxResult);

        // 4. Caculate Parameters for later
        auto weightParams = ImageUtils::precalculateDarkWeightParams(
            minL, maxL, LOWER_THRESHOLD_PERCENT, UPPER_THRESHOLD_PERCENT);

        // clang-format off
        // 5. Interation through the image
        #pragma omp parallel for
        // clang-format on

        for (int y = 0; y < srcImg.rows; y++) {
            // 5.2 Get the pointers of the first pixel each line
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

            // clang-format off
            #pragma omp simd
            // clang-format on
            // 5.2 Calculate pixel-wise
            for (int x = 0; x < srcImg.cols; x++) {
                // 5.2.1 Extract and normalize the current Value
                float currVal = srcPtr[x] * invMaxRange;

                float weight = ImageUtils::calculateDarkWeight(currVal, weightParams);
                float blackChange = weight * blackFactor;

                float newVal = std::clamp(currVal + blackChange, 0.0f, 1.0f);

                dstPtr[x] = static_cast<T>(newVal * maxRange);
            }
        }
        return dstImg;
    }
};