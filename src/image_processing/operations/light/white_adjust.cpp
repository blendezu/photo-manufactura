#include "white_adjust.h"

#include <opencv2/core/hal/interface.h>

#include <opencv2/core/mat.hpp>

#include "color_space.h"
#include "image_utils.h"

cv::Mat AdjustWhite::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in AdjustWhite: empty image\n";
        return cv::Mat();
    }

    float whiteFactor = white / 800.0f;

    if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        auto start = std::chrono::high_resolution_clock::now();

        auto minMaxVal = ImageUtils::calculateMinMax(hslImg, 2);
        float minL = std::get<0>(minMaxVal);
        float maxL = std::get<1>(minMaxVal);

        auto weightParams = ImageUtils::precalculateWhiteWeightParams(minL, maxL, 0.7, 0.9);

#pragma omp parallel for
        for (int y = 0; y < hslImg.rows; y++) {
            float* __restrict hslPtr = hslImg.ptr<float>(y);

            int len = hslImg.cols * 3;

            for (int x = 2; x < len; x += 3) {
                float currL = hslPtr[x];

                float weight = ImageUtils::calculateBrightWeight(currL, weightParams);

                float whiteChange = weight * whiteFactor;
                float newL = std::clamp(currL + whiteChange, 0.0f, 1.0f);

                hslPtr[x] = newL;
            }
        }
        if (srcImg.depth() == CV_8U) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            std::cout << duration.count() << std::endl;
            return ColorSpace::convertHSL2BGR(hslImg, 8);
        } else {
            return ColorSpace::convertHSL2BGR(hslImg, 16);
        }
    }

    else if (srcImg.type() == CV_8UC1) {
        return whiteGrayImgTemplate<uchar>(srcImg, whiteFactor);
    }

    else if (srcImg.type() == CV_16UC1) {
        return whiteGrayImgTemplate<ushort>(srcImg, whiteFactor);
    }

    else {
        return cv::Mat();
    }
}