#pragma once
#include <algorithm>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <string>

#include "operation_base.h"

class AdjustShadow : public ImageOperation {
   private:
    int shadow;

   public:
    AdjustShadow(int value) : shadow(value) {}

    // apply this function on the image
    cv::Mat apply(const cv::Mat& srcImg) override;

    // name for the function on the GUI
    std::string getName() const override {
        return "Shadow";
    }

    std::string getSettings() const override {
        return "Shadow: " + std::to_string(shadow);
    }

    void setShadow(int value) {
        shadow = value;
    }

    int getShadow() {
        return shadow;
    }

   private:
    float caculateWeight(float currL) {
        float weight = 0.0f;
        const float L1 = 0.3f;  //
        const float L2 = 0.6f;

        if (currL <= L1) {
            weight = 1.0f;
        } else if (currL <= L2) {
            // linear reduction von 1 -> 0
            float t = (currL - L1) / (L2 - L1);
            weight = 1.0f - t * t;
        } else {
            weight = 0.0f;
        }

        return std::clamp(weight, 0.0f, 1.0f);
    }

    template <typename T>
    cv::Mat shadowGrayImgTemplate(const cv::Mat& srcImg, float shadowFactor) {
        if (srcImg.empty()) {
            std::cerr << "Error: empty input image\n";
            return cv::Mat();
        }

        float maxValue = 0.0f;
        if (srcImg.type() == CV_8UC1) {
            maxValue = 255.0f;
        } else if (srcImg.type() == CV_16UC1) {
            maxValue = 65535.0f;
        }

        cv::Mat dstImg(srcImg.size(), srcImg.type());  // output image

        for (int y = 0; y < srcImg.rows; y++) {
            const T* srcPtr = srcImg.ptr<T>(y);
            T* dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                // float normPixel = static_cast<float>(srcPtr[x]) / maxValue;  // 0 -> 1
                float normPixel = srcPtr[x] / maxValue;  // 0 -> 1
                float weight = caculateWeight(normPixel);
                float brightnessChange = weight * shadowFactor;

                float newPixel = normPixel + brightnessChange;
                newPixel = std::clamp(newPixel, 0.0f, 1.0f);

                dstPtr[x] = static_cast<T>(newPixel * maxValue);
            }
        }
        return dstImg;
    }
};