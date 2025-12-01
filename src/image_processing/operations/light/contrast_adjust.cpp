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
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        auto start = std::chrono::high_resolution_clock::now();

        int len = hslImg.cols * 3;  // --> 1D array

        for (int y = 0; y < hslImg.rows; y++) {
            float* __restrict hslPtr = hslImg.ptr<float>(y);

            for (int x = 2; x < len; x += 3) {
                float currL = hslPtr[x];
                float newL = (currL - 0.5f) * contrastFactor + 0.5f;
                newL = std::clamp(newL, 0.0f, 1.0f);

                hslPtr[x] = newL;
            }
        }
        if (srcImg.type() == CV_16UC3) {
            return ColorSpace::convertHSL2BGR(hslImg, 16);
        } else {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << duration.count() << std::endl;
            return ColorSpace::convertHSL2BGR(hslImg, 8);
        }
    } else {
        std::cerr << "Error: unsupported image type";
        return cv::Mat();
    }
}