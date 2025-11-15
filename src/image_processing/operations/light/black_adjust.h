#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <string>

#include "color_space.h"
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

    template <typename T>
    cv::Mat blackRGBImgTemplate(const cv::Mat& srcImg, float blackFactor) {
        if (srcImg.type() != CV_8UC3 && srcImg.type() != CV_16UC3) {
            std::cerr << "Error in blackRGBImageTemplate: unsupported image type\n";
            return cv::Mat();
        }
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);
        cv::Mat dstImg(hslImg.size(), hslImg.type());

        auto [minL, maxL] = ImageUtils::caculateMinMax(hslImg, 2);

        for (int y = 0; y < srcImg.rows; y++) {
            const cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);
            cv::Vec3f* dstPtr = dstImg.ptr<cv::Vec3f>(y);

            for (int x = 0; x < hslImg.cols; x++) {
                float H = hslPtr[x][0];
                float S = hslPtr[x][1];
                float currL = hslPtr[x][2];

                float weight = ImageUtils::caculateDarkWeight(currL, minL, maxL, 0.1f, 0.3f);
                float newL = currL + weight * blackFactor;

                newL = std::clamp(newL, 0.0f, 1.0f);

                dstPtr[x] = cv::Vec3f(H, S, newL);
            }
        }
        if (srcImg.depth() == CV_16U)
            return ColorSpace::convertHSL2BGR(dstImg, 16);
        else {
            return ColorSpace::convertHSL2BGR(dstImg, 8);
        }
    }
};