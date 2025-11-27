#include "vibrance_adjust.h"

#include <algorithm>
#include <opencv2/core/mat.hpp>

#include "color_space.h"

cv::Mat AdjustVibrance::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in AdjustVibrance: empty input image\n";
        return cv::Mat();
    }

    if (srcImg.channels() == 1) {
        return srcImg;
    }

    float vibranceFactor = vibrance / 300.0f;

    // convert input image to hsl
    cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

    int len = hslImg.cols * 3;  // 1D Array

#pragma omp parallel for
    for (int y = 0; y < hslImg.rows; y++) {
        float* __restrict hslPtr = hslImg.ptr<float>(y);

        for (int x = 1; x < len; x += 3) {
            float currS = hslPtr[x];

            float weight = caculateWeight(currS);
            float newS = currS + weight * vibranceFactor;
            newS = std::clamp(newS, 0.0f, 1.0f);

            hslPtr[x] = newS;
        }
    }

    if (srcImg.depth() == CV_8U) {
        return ColorSpace::convertHSL2BGR(hslImg, 8);
    } else {
        return ColorSpace::convertHSL2BGR(hslImg, 16);
    }
}