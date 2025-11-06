#include <algorithm>
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
                float floatVal = srcPtr[x] / maxRange;

                float weight = ImageUtils::caculateDarkWeight(floatVal, minVal, maxVal, 0.1, 0.3);
                float blackChange = weight * blackFactor;

                float newFloatVal = std::clamp(floatVal + blackChange, 0.0f, 1.0f);

                dstPtr[x] = static_cast<T>(newFloatVal * maxRange);
            }
        }
        return dstImg;
    }
};