#include "highlight_adjust.h"

#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

#include "color_space.h"
#include "image_utils.h"

cv::Mat AdjustHighlight::apply(const cv::Mat& srcImg) {
    float highlightFactor = highlight / 800.0f;

    // color image
    if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        // 1. Convert to HSL
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        // find the min & max value to caculate the weight
        auto minMaxVal = ImageUtils::calculateMinMax(hslImg, 2);
        float minL = std::get<0>(minMaxVal);
        float maxL = std::get<1>(minMaxVal);

        // to caculate the weight
        auto weightParams = ImageUtils::precalculateWhiteWeightParams(minL, maxL, 0.4, 0.7);

        int len = hslImg.cols * 3;  // 1D Array

        // clang-format off
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < srcImg.rows; y++) {
            float* __restrict hslPtr = hslImg.ptr<float>(y);

            for (int x = 2; x < len; x += 3) {
                float currL = hslPtr[x];

                float weight = ImageUtils::calculateBrightWeight(currL, weightParams);
                float brightnessChange = weight * highlightFactor;
                float newL = std::clamp(currL + brightnessChange, 0.0f, 1.0f);

                hslPtr[x] = newL;
            }
        }
        if (srcImg.depth() == CV_8U) {
            return ColorSpace::convertHSL2BGR(hslImg, 8);
        } else {
            return ColorSpace::convertHSL2BGR(hslImg, 16);
        }
    }  // 8 bit gray image
    else if (srcImg.type() == CV_8UC1) {
        return highlightGrayImgTemplate<uchar>(srcImg, highlightFactor);
    }

    // 16 bit gray image
    else if (srcImg.type() == CV_16UC1) {
        return highlightGrayImgTemplate<ushort>(srcImg, highlightFactor);
    }

    else {
        std::cerr << "Error: unsupported image type\n";
        return cv::Mat();
    }
}