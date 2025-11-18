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

    std::cout << "RGB\n";
    ColorSpace ColorSpace;
    cv::Mat hslImg = ColorSpace.convertBGR2HSL(srcImg);  // convert to hsl image
    cv::Mat dstImg(hslImg.size(), hslImg.type());        // output image
    float satFactor = 1 + saturation / 100.0f;  // change factor <1 --> reduce; >1 --> increase

    for (int y = 0; y < hslImg.rows; y++) {
        const cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);
        cv::Vec3f* dstPtr = dstImg.ptr<cv::Vec3f>(y);

        for (int x = 0; x < hslImg.cols; x++) {
            float H = hslPtr[x][0];
            float currS = hslPtr[x][1];
            float L = hslPtr[x][2];

            float newS = currS * satFactor;
            newS = std::clamp(newS, 0.0f, 1.0f);
            dstPtr[x] = cv::Vec3f(H, newS, L);
        }
    }
    if (srcImg.depth() == CV_8U) {
        return ColorSpace.convertHSL2BGR(dstImg, 8);

    } else {
        return ColorSpace.convertHSL2BGR(dstImg, 16);
    }
}