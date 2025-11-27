#include "white_balance.h"

#include <opencv2/core/hal/interface.h>

#include <opencv2/core/mat.hpp>

cv::Mat WhiteBalance::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in WhiteBalance: empty input image\n";
        cv::Mat();
    }
    float changeFactorR = 1 + temp / 200.0f;
    float changeFactorB = 1 - temp / 200.0f;

    if (srcImg.type() == CV_8UC1 || srcImg.type() == CV_16UC1) {
        return srcImg;
    }

    else if (srcImg.type() == CV_8UC3) {
        return whiteBalanceTemplate<uchar>(srcImg, changeFactorR, changeFactorB);
    }

    else if (srcImg.type() == CV_16UC3) {
        return whiteBalanceTemplate<ushort>(srcImg, changeFactorR, changeFactorB);
    } else {
        std::cerr << "Error in WhiteBalance: unsupported image type\n";
        return cv::Mat();
    }
}