#pragma once
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <string>

#include "image_utils.h"
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
    template <typename T>
    cv::Mat shadowGrayImgTemplate(const cv::Mat& srcImg, float shadowFactor) {
        if (srcImg.empty()) {
            std::cerr << "Error: empty input image\n";
            return cv::Mat();
        }

        float invMaxRange = 0.0f;  // --> to avoid division in the for loop
        float maxRange = 0.0f;
        if (srcImg.type() == CV_8UC1) {
            invMaxRange = 1 / 255.0f;
            maxRange = 255.0f;
        } else if (srcImg.type() == CV_16UC1) {
            invMaxRange = 1 / 65535.0f;
            maxRange = 65535.0f;
        }

        cv::Mat dstImg(srcImg.size(), srcImg.type());  // output image

        auto minMaxVal = ImageUtils::calculateMinMax(srcImg, 0);
        float minVal = std::get<0>(minMaxVal);
        float maxVal = std::get<1>(minMaxVal);

        auto weightParams = ImageUtils::precalculateDarkWeightParams(minVal, maxVal, 0.3, 0.6);

#pragma omp parallel for
        for (int y = 0; y < srcImg.rows; y++) {
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                // float normPixel = static_cast<float>(srcPtr[x]) / maxValue;  // 0 -> 1
                float currVal = srcPtr[x] * invMaxRange;  // 0 -> 1
                float weight = ImageUtils::calculateDarkWeight(currVal, weightParams);
                float brightnessChange = weight * shadowFactor;

                float newPixel = currVal + brightnessChange;
                newPixel = std::clamp(newPixel, 0.0f, 1.0f);

                dstPtr[x] = static_cast<T>(newPixel * maxRange);
            }
        }
        return dstImg;
    }
};