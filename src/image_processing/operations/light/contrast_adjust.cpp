#include "contrast_adjust.h"

#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/saturate.hpp>

#include "../utils/color_space.h"

cv::Mat AdjustContrast::apply(const cv::Mat& srcImg) {
    float contrastFactor = 1.0f + contrast / 100.0f;

    if (srcImg.type() == CV_8UC1) {
        cv::Mat dstImg(srcImg.size(), srcImg.type());  // output image

        for (int y = 0; y < srcImg.rows; y++) {
            const uchar* srcPtr = srcImg.ptr<uchar>(y);
            uchar* dstPtr = dstImg.ptr<uchar>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                int newVal = static_cast<int>((srcPtr[x] - 128) * contrastFactor + 128);
                dstPtr[x] = cv::saturate_cast<uchar>(newVal);
            }
        }
        return dstImg;

    }

    else if (srcImg.type() == CV_16UC1) {
        cv::Mat dstImg(srcImg.size(), srcImg.type());  // output image

        for (int y = 0; y < srcImg.rows; y++) {
            const ushort* srcPtr = srcImg.ptr<ushort>(y);
            ushort* dstPtr = dstImg.ptr<ushort>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                int newVal = static_cast<int>((srcPtr[x] - 32768) * contrastFactor + 32768);
                dstPtr[x] = cv::saturate_cast<ushort>(newVal);
            }
        }
        return dstImg;
    }

    else if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        ColorSpace ColorSpace;
        cv::Mat hslImg = ColorSpace.convertBGR2HSL(srcImg);
        cv::Mat dstImg(hslImg.size(), hslImg.type());

        for (int y = 0; y < hslImg.rows; y++) {
            const cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);
            cv::Vec3f* dstPtr = dstImg.ptr<cv::Vec3f>(y);

            for (int x = 0; x < hslImg.cols; x++) {
                float H = hslPtr[x][0];
                float S = hslPtr[x][1];
                float currL = hslPtr[x][2];
                float newL = (currL - 0.5f) * contrastFactor + 0.5f;
                newL = std::clamp(newL, 0.0f, 1.0f);

                dstPtr[x] = cv::Vec3f(H, S, newL);
            }
        }
        if (srcImg.type() == CV_16UC3) {
            return ColorSpace.convertHSL2BGR(dstImg, 16);
        } else {
            return ColorSpace.convertHSL2BGR(dstImg, 8);
        }
    } else {
        std::cerr << "Error: unsupported image type";
        return cv::Mat();
    }
}