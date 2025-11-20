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
        cv::Mat dstImg(hslImg.size(), hslImg.type());
        auto [minL, maxL] = ImageUtils::caculateMinMax(hslImg, 2);

        for (int y = 0; y < hslImg.rows; y++) {
            const cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);
            cv::Vec3f* dstPtr = dstImg.ptr<cv::Vec3f>(y);

            for (int x = 0; x < hslImg.cols; x++) {
                float H = hslPtr[x][0];
                float S = hslPtr[x][1];
                float currL = hslPtr[x][2];

                float weight = ImageUtils::caculateBrightWeight(currL, minL, maxL, 0.7, 0.9);

                float whiteChange = weight * whiteFactor;
                float newL = std::clamp(currL + whiteChange, 0.0f, 1.0f);

                dstPtr[x] = cv::Vec3f(H, S, newL);
            }
        }
        if (srcImg.depth() == CV_8U) {
            return ColorSpace::convertHSL2BGR(dstImg, 8);
        } else {
            return ColorSpace::convertHSL2BGR(dstImg, 16);
        }
    }

    else if (srcImg.type() == CV_8UC1) {
        return whiteGrayImgTemplate<uchar>(srcImg, whiteFactor);
    }

    else {
        return cv::Mat();
    }
}