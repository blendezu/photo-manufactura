#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <string>

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
    float caculateWeight(float currL, float minL, float maxL) {
        float topL = minL + 0.7f * (maxL - minL);
        float midL = minL + 0.4f * (maxL - minL);

        float weight = 0.0f;

        if (currL >= topL) {
            weight = 1.0f;
        } else if (currL >= midL) {
            float t = (currL - midL) / (topL - midL);
            weight = t * t * (3 - 2 * t);
        } else {
            weight = 0.0f;
        }
        return weight;
    }

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
        float rangeMax = 0.0f;
        if (srcImg.depth() == CV_8U) {
            rangeMax = 255.0f;
        } else {
            rangeMax = 65535.0f;
        }

        double imgMin, imgMax;
        cv::minMaxLoc(srcImg, &imgMin, &imgMax);

        float minL = imgMin / rangeMax;
        float maxL = imgMax / rangeMax;

        for (int y = 0; y < srcImg.rows; y++) {
            const T* srcPtr = srcImg.ptr<T>(y);
            T* dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                float normedValue = srcPtr[x] / rangeMax;
                float weight = caculateWeight(normedValue, minL, maxL);

                float deltaL = weight * changeFactor;
                float newL = std::clamp(normedValue + deltaL, 0.0f, 1.0f);

                dstPtr[x] = static_cast<T>(newL * rangeMax);
            }
        }
        return dstImg;
    }
};