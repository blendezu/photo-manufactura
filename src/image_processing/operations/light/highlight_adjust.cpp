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
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);
        cv::Mat dstImg(hslImg.size(), hslImg.type());

        auto [minL, maxL] = ImageUtils::caculateMinMax(hslImg, 2);
        std::cout << minL << "; " << maxL << std::endl;
        for (int y = 0; y < srcImg.rows; y++) {
            const cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);
            cv::Vec3f* dstPtr = dstImg.ptr<cv::Vec3f>(y);

            for (int x = 0; x < hslImg.cols; x++) {
                float H = hslPtr[x][0];
                float S = hslPtr[x][1];
                float currL = hslPtr[x][2];

                float weight = caculateWeight(currL, minL, maxL);
                float brightnessChange = weight * highlightFactor;
                float newL = std::clamp(currL + brightnessChange, 0.0f, 1.0f);

                dstPtr[x] = cv::Vec3f(H, S, newL);
            }
        }
        if (srcImg.depth() == CV_8U) {
            return ColorSpace::convertHSL2BGR(dstImg, 8);
        } else {
            return ColorSpace::convertHSL2BGR(dstImg, 16);
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