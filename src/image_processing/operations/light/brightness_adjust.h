#ifndef BRIGHTNESS_ADJUST_H
#define BRIGHTNESS_ADJUST_H

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <string>

#include "operation_base.h"

class AdjustBrightness : public HalideOperation {
   private:
    int m_brightness;  // -100 --> 100

    // --- Constant Parameters ---
   public:
    static constexpr float BRIGHTNESS_SCALING_FACTOR = 100.0f;

   private:
    // --- Halide Runtime Parameters ---
    Halide::Param<float> p_changeFactor{"brightnessFactor"};
    Halide::Param<float> p_maxRange{"p_brightness_maxRange"};
    Halide::Param<float> p_depthScale{"depthScale"};
    Halide::Param<float> p_brightness{"brightness"};
    Halide::Param<float> p_minL{"p_brightness_minL"};
    Halide::Param<float> p_maxL{"p_brightness_maxL"};

   public:
    AdjustBrightness(int value) : m_brightness(std::clamp(value, -100, 100)) {
        p_changeFactor.set(0.0f);
        p_maxRange.set(255.0f);
        p_depthScale.set(255.0f);
        p_brightness.set(0.0f);
        p_minL.set(0.0f);
        p_maxL.set(1.0f);
    }
    // bool supportsHalide() const override {
    //     return true;
    // }

    bool requiresFreshStats() const override {
        return true;
    }
    void prepareParameters(const cv::Mat& srcImg) override;

    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Brightness";
    }

    std::string getSettings() const override {
        return "brightness: " + std::to_string(m_brightness);
    }

    void setBrightness(int value) {
        m_brightness = std::clamp(value, -100, 100);
    }

    int getBrightness() {
        return m_brightness;
    }

   private:
    template <typename T>
    cv::Mat grayImgTemplate(const cv::Mat& srcImg, float changeFactor, float minL, float maxL) {
        // 1. Determine max value for normalization
        float maxVal = (srcImg.depth() == CV_8U) ? 255.0f : 65535.0f;
        float invRange = 1.0f / (maxL - minL + 0.0001f);

        // 2. Create the Destination Image
        cv::Mat dstImg(srcImg.size(), srcImg.type());

        // clang-format off
        // 3. Iteration through the Image
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < srcImg.rows; y++) {
            // 3.1 Get the pointers of the first pixel each line
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

            // 3.1 Assign new Value pixel-wise
            for (int x = 0; x < srcImg.cols; x++) {
                // Normalize to [0, 1] relative to image min/max
                float val = static_cast<float>(srcPtr[x]) / maxVal;  // Global 0..1

                // Curve calculation
                float l_norm = (val - minL) * invRange;
                float weight = 4.0f * l_norm * (1.0f - l_norm);
                // Clamp weight
                if (weight < 0.0f)
                    weight = 0.0f;

                float delta = weight * changeFactor;
                float newVal = std::clamp(val + delta, 0.0f, 1.0f);

                dstPtr[x] = cv::saturate_cast<T>(newVal * maxVal);
            }
        }

        return dstImg;
    }
};

#endif