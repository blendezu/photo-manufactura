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
    float BRIGHTNESS_SCALING_FACTOR = 100.0f;

    // --- Halide Runtime Parameters ---
    Halide::Param<float> p_changeFactor{"brightnessFactor"};
    Halide::Param<float> p_maxRange{"maxRange"};
    Halide::Param<float> p_depthScale{"depthScale"};
    Halide::Param<float> p_brightness{"brightness"};

   public:
    AdjustBrightness(int value) : m_brightness(std::clamp(value, -100, 100)) {
        p_changeFactor.set(0.0f);
        p_maxRange.set(255.0f);
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
    cv::Mat grayImgTemplate(const cv::Mat& srcImg, int brightness) {
        // 1. Calculate delta Value if the Image is 16 bit
        int deltaVal = 0;
        (srcImg.depth() == CV_8U) ? deltaVal = brightness : deltaVal = brightness * 256;

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
                dstPtr[x] = cv::saturate_cast<T>(srcPtr[x] + deltaVal);
            }
        }

        return dstImg;
    }
};

#endif