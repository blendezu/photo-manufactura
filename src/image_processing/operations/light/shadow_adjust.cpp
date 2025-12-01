#include "shadow_adjust.h"

#include <opencv2/core/hal/interface.h>

#include <iostream>
#include <opencv2/core/mat.hpp>

#include "color_space.h"
#include "image_utils.h"

cv::Mat AdjustShadow::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in AjustShadow: empty iput image\n";
        return cv::Mat();
    }

    float shadowFactor = shadow / 800.0f;

    if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        auto minMaxVal = ImageUtils::calculateMinMax(hslImg, 2);
        float minL = std::get<0>(minMaxVal);
        float maxL = std::get<1>(minMaxVal);

        auto weightParams = ImageUtils::precalculateDarkWeightParams(minL, maxL, 0.3, 0.6);

        int len = hslImg.cols * 3;
#pragma omp parallel for
        for (int y = 0; y < hslImg.rows; y++) {
            float* __restrict hslPtr = hslImg.ptr<float>(y);

            for (int x = 2; x < len; x += 3) {
                float currL = hslPtr[x];

                float weight = ImageUtils::calculateDarkWeight(currL, weightParams);
                float brightnessChange = weight * shadowFactor;

                float newL = currL + brightnessChange;
                newL = std::clamp(newL, 0.0f, 1.0f);

                hslPtr[x] = newL;
            }
        }
        if (srcImg.type() == CV_8UC3) {
            std::cout << "Starting clock ...\n";
            return ColorSpace::convertHSL2BGR(hslImg, 8);
        } else {
            return ColorSpace::convertHSL2BGR(hslImg, 16);
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