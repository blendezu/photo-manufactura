#include "black_adjust.h"

#include <opencv2/core/hal/interface.h>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

#include "color_space.h"

cv::Mat AdjustBlack::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in AdjustBlack: empty input image\n";
        return cv::Mat();
    }

    float blackFactor = black / 800.0f;
    if (srcImg.type() == CV_8UC1) {
        return blackGrayImgTemplate<uchar>(srcImg, blackFactor);
    }

    else if (srcImg.type() == CV_16UC1) {
        return blackGrayImgTemplate<ushort>(srcImg, blackFactor);
    }

    else if (srcImg.type() == CV_16UC3 || srcImg.type() == CV_8UC3) {
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        auto minMaxVal = ImageUtils::calculateMinMax(hslImg, 2);
        float minL = std::get<0>(minMaxVal);
        float maxL = std::get<1>(minMaxVal);

        auto weightParams = ImageUtils::precalculateDarkWeightParams(minL, maxL, 0.1f, 0.3f);
        int len = hslImg.cols * 3;

#pragma omp parallel for
        for (int y = 0; y < srcImg.rows; y++) {
            float* __restrict hslPtr = hslImg.ptr<float>(y);

            // #pragma omp simd
            for (int x = 2; x < len; x += 3) {
                float currL = hslPtr[x];

                float weight = ImageUtils::calculateDarkWeight(currL, weightParams);
                float newL = currL + weight * blackFactor;

                newL = std::clamp(newL, 0.0f, 1.0f);

                hslPtr[x] = newL;
            }
        }
        if (srcImg.depth() == CV_16U)
            return ColorSpace::convertHSL2BGR(hslImg, 16);
        else {
            return ColorSpace::convertHSL2BGR(hslImg, 8);
        }
    }

    else {
        std::cerr << "Error in AdjustBlack: unsupported image type\n";
        return cv::Mat();
    }
}