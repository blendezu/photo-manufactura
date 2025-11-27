#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <string>

#include "image_utils.h"
#include "operation_base.h"

class AdjustBlack : public ImageOperation {
   private:
    int black;

   public:
    AdjustBlack(int value) : black(value) {}

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Black";
    }

    std::string getSettings() const override {
        return "black: " + std::to_string(black);
    }

    void setBlack(int value) {
        black = std::clamp(value, -100, 100);
    }

    int getBlack() {
        return black;
    }

   private:
    template <typename T>
    cv::Mat blackGrayImgTemplate(const cv::Mat& srcImg, float blackFactor) {
        cv::Mat dstImg(srcImg.size(), srcImg.type());

        float maxRange = 0.0f;
        (srcImg.depth() == CV_8U) ? maxRange = 255.0f : maxRange = 65535.0f;  // 8 bit or 16 bit
        const float invMaxRange = 1.0f / maxRange;  // to avoid division in the for loop

        auto minMaxResult = ImageUtils::calculateMinMax(srcImg, 0);
        float minL = std::get<0>(minMaxResult);
        float maxL = std::get<1>(minMaxResult);

        auto weightParams = ImageUtils::precalculateDarkWeightParams(minL, maxL, 0.1f, 0.3f);

#pragma omp parallel for
        for (int y = 0; y < srcImg.rows; y++) {
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

#pragma omp simd
            for (int x = 0; x < srcImg.cols; x++) {
                float floatVal = srcPtr[x] * invMaxRange;

                float weight = ImageUtils::calculateDarkWeight(floatVal, weightParams);
                float blackChange = weight * blackFactor;

                float newFloatVal = std::clamp(floatVal + blackChange, 0.0f, 1.0f);

                dstPtr[x] = static_cast<T>(newFloatVal * maxRange);
            }
        }
        return dstImg;
    }
};