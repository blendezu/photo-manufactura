#include "brightness_adjust.h"

#include <opencv2/core/mat.hpp>
#include <opencv2/core/saturate.hpp>
#include <opencv2/opencv.hpp>

#include "color_space.h"

cv::Mat BrightnessAdjust::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in BrightnessAdjust: the input image is empty\n";
        return cv::Mat();
    }

    // RGB image
    if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        ColorSpace ColorSpace;
        cv::Mat hslImg = ColorSpace.convertBGR2HSL(srcImg);

        cv::Mat dstImg(hslImg.size(), hslImg.type());  // output image

        float deltaL = brightness / 100.0f;

        for (int y = 0; y < hslImg.rows; y++) {
            const cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);
            cv::Vec3f* dstPtr = dstImg.ptr<cv::Vec3f>(y);

            for (int x = 0; x < hslImg.cols; x++) {
                float H = hslPtr[x][0];
                float S = hslPtr[x][1];
                float currL = hslPtr[x][2];
                float newL = std::clamp(currL * (1.0f + deltaL), 0.0f, 1.0f);

                dstPtr[x] = cv::Vec3f(H, S, newL);
            }
        }
        if (srcImg.type() == CV_8UC3) {
            return ColorSpace.convertHSL2BGR(dstImg, 8);
        } else {
            return ColorSpace.convertHSL2BGR(dstImg, 16);
        }
    }

    // gray image
    else if (srcImg.type() == CV_8UC1) {
        cv::Mat dstImg(srcImg.size(), srcImg.type());
        for (int y = 0; y < srcImg.rows; y++) {
            const uchar* srcPtr = srcImg.ptr<uchar>(y);
            uchar* dstPtr = dstImg.ptr<uchar>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                dstPtr[x] = std::clamp(srcPtr[x] + brightness, 0, 255);
            }
        }
        return dstImg;
    }

    else if (srcImg.type() == CV_16UC1) {
        cv::Mat dstImg(srcImg.size(), srcImg.type());
        for (int y = 0; y < srcImg.rows; y++) {
            const ushort* srcPtr = srcImg.ptr<ushort>(y);
            ushort* dstPtr = dstImg.ptr<ushort>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                int adjusted = srcPtr[x] + static_cast<int>(brightness);
                dstPtr[x] = cv::saturate_cast<ushort>(adjusted);
            }
        }
        return dstImg;
    }

    else {
        std::cerr << "Error: unsupported image type\n";
        return cv::Mat();
    }
}