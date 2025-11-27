#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <string>

#include "image_utils.h"
#include "operation_base.h"

class AdjustHighlight : public ImageOperation {
   private:
    int highlight;

   public:
    AdjustHighlight(int value) : highlight(value) {}

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Highlight";
    }

    std::string getSettings() const override {
        return "highlight: " + std::to_string(highlight);
    }

    void setHighlight(int value) {
        highlight = std::clamp(value, -100, 100);
    }

    int getHighlight() {
        return highlight;
    }

   private:
    template <typename T>
    cv::Mat highlightGrayImgTemplate(const cv::Mat srcImg, float changeFactor) {
        if (srcImg.empty()) {
            std::cerr << "Error in AdjustHighlight: empty input image\n";
            return cv::Mat();
        }

        if (srcImg.type() != CV_8UC1 && srcImg.type() != CV_16UC1) {
            std::cerr << "Error in AdjustHighlight: expect only gray image\n";
        }

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

        // find min max value to calculate weight
        auto minMaxVal = ImageUtils::calculateMinMax(srcImg, 0);
        float minL = std::get<0>(minMaxVal);
        float maxL = std::get<1>(minMaxVal);

        auto weightParams = ImageUtils::precalculateWhiteWeightParams(minL, maxL, 0.4f, 0.7f);

        // clang-format off
        #pragma omp parallel for
        // clang-format on

        for (int y = 0; y < srcImg.rows; y++) {
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                float currVal = srcPtr[x] * invMaxRange;
                float weight = ImageUtils::calculateBrightWeight(currVal, weightParams);

                float deltaL = weight * changeFactor;
                float newL = std::clamp(currVal + deltaL, 0.0f, 1.0f);

                dstPtr[x] = static_cast<T>(newL * maxRange);
            }
        }
        return dstImg;
    }
};