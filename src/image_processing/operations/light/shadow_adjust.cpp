#include "shadow_adjust.h"

#include <opencv2/core/hal/interface.h>

#include <cmath>
#include <iostream>
#include <opencv2/core/mat.hpp>

#include "color_space.h"

cv::Mat AdjustShadow::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in AjustShadow: empty iput image\n";
        return cv::Mat();
    }

    float shadowFactor = shadow / 800.0f;

    if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
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

                float gewicht = caculateWeight(currL);
                float brightnessChange = gewicht * shadowFactor;

                float newL = currL + brightnessChange;
                newL = std::clamp(newL, 0.0f, 1.0f);

                dstPtr[x] = cv::Vec3f(H, S, newL);
            }
        }
        if (srcImg.depth() == 8U) {
            return ColorSpace.convertHSL2BGR(dstImg, 8);
        } else {
            return ColorSpace.convertHSL2BGR(dstImg, 16);
        }
    }

    else if (srcImg.type() == CV_8UC1) {
        return shadowGrayImgTemplate<uchar>(srcImg, shadowFactor);
    }

    else if (srcImg.type() == CV_16UC1) {
        return shadowGrayImgTemplate<ushort>(srcImg, shadowFactor);
    } else {
        return cv::Mat();
    }
}