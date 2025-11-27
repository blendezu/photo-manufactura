#ifndef BRIGHTNESS_ADJUST_H
#define BRIGHTNESS_ADJUST_H

#include <opencv2/core/hal/interface.h>

#include <string>

#include "operation_base.h"

class AdjustBrightness : public ImageOperation {
   private:
    int brightness;  // -100 --> 100

   public:
    AdjustBrightness(int value) : brightness(std::clamp(value, -100, 100)) {}

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Brightness";
    }

    std::string getSettings() const override {
        return "brightness: " + std::to_string(brightness);
    }

    void setBrightness(int value) {
        brightness = std::clamp(value, -100, 100);
    }

    int getBrightness() {
        return brightness;
    }

   private:
    template <typename T>
    cv::Mat grayImgTemplate(const cv::Mat& srcImg, int brightness) {
        int change = 0;
        (srcImg.depth() == CV_8U) ? change = brightness : change = brightness * 256;

        cv::Mat dstImg(srcImg.size(), srcImg.type());  // output image
        // #pragma omp parallel for --> no parallelism because Memory Bound, simd enough
        for (int y = 0; y < srcImg.rows; y++) {
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                dstPtr[x] = cv::saturate_cast<T>(srcPtr[x] + change);
            }
        }
        return dstImg;
    }
};

#endif