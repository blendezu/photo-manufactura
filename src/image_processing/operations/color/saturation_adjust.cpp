#include "saturation_adjust.h"

#include <algorithm>
#include <iostream>
#include <opencv2/core/mat.hpp>

#include "color_space.h"

cv::Mat AdjustSaturation::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in AdjustSaturation: empty image\n";
        return cv::Mat();
    }

    if (srcImg.channels() == 1) {
        return srcImg;
    }

    cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);  // convert to hsl image
    float satFactor = 1 + saturation / 100.0f;  // change factor <1 --> reduce; >1 --> increase

    int len = hslImg.cols * 3;

#pragma omp parallel for
    for (int y = 0; y < hslImg.rows; y++) {
        float* __restrict hslPtr = hslImg.ptr<float>(y);

        for (int x = 1; x < len; x += 3) {
            float currS = hslPtr[x];

            float newS = currS * satFactor;
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