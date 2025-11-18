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
    ColorSpace ColorSpace;
    cv::Mat hslImg = ColorSpace.convertBGR2HSL(srcImg);  // convert input image to hsl
    cv::Mat dstImg(hslImg.size(), hslImg.type());

    for (int y = 0; y < hslImg.rows; y++) {
        const cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);
        cv::Vec3f* dstPtr = dstImg.ptr<cv::Vec3f>(y);

        for (int x = 0; x < hslImg.cols; x++) {
            float H = hslPtr[x][0];
            float currS = hslPtr[x][1];
            float L = hslPtr[x][2];

            float weight = caculateWeight(currS);
            float newS = currS + weight * vibranceFactor;
            newS = std::clamp(newS, 0.0f, 1.0f);

            dstPtr[x] = cv::Vec3f(H, newS, L);
        }
    }

    if (srcImg.depth() == CV_8U) {
        std::cout << "8";
        return ColorSpace.convertHSL2BGR(dstImg, 8);
    } else {
        std::cout << "16";
        return ColorSpace.convertHSL2BGR(dstImg, 16);
    }
}