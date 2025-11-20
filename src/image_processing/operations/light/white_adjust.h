#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <opencv2/core/mat.hpp>
#include <string>

#include "image_utils.h"
#include "operation_base.h"

class AdjustWhite : public ImageOperation {
   private:
    int white;

   public:
    AdjustWhite(int value) : white(value) {}

    cv::Mat apply(const cv::Mat& scrImg) override;

    std::string getName() const override {
        return "White";
    }

    std::string getSettings() const override {
        return "white: " + std::to_string(white);
    }

    void setWhite(int value) {
        white = std::clamp(value, -100, 100);
    }

    int getWhite() {
        return white;
    }

   private:
    template <typename T>
    cv::Mat whiteGrayImgTemplate(const cv::Mat& srcImg, float whiteFactor) {
        cv::Mat dstImg(srcImg.size(), srcImg.type());

        float maxRange = 0.0f;
        if (srcImg.depth() == CV_8U) {
            maxRange = 255.0f;
        } else {
            maxRange = 65535.0f;
        }
        auto [minVal, maxVal] = ImageUtils::caculateMinMax(srcImg, 0);

        for (int y = 0; y < srcImg.rows; y++) {
            const T* srcPtr = srcImg.ptr<T>(y);
            T* dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                float normedVal = srcPtr[x] / maxRange;

                float weight =
                    ImageUtils::caculateBrightWeight(normedVal, minVal, maxVal, 0.7, 0.9);

                float whiteChange = weight * whiteFactor;
                float newFValue = std::clamp(normedVal + whiteChange, 0.0f, 1.0f);

                dstPtr[x] = static_cast<T>(newFValue * maxRange);
            }
        }
        return dstImg;
    }
};