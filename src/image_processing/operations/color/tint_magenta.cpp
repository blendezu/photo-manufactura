#include "tint_magenta.h"

#include <opencv2/core/hal/interface.h>

#include <opencv2/core/mat.hpp>

cv::Mat TintMagenta::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in TintMagenta: empty input image\n";
        return cv::Mat();
    }

    float changeFactor = 1 - tint / 200.0f;

    if (srcImg.type() == CV_8UC1 || srcImg.type() == CV_16UC1) {
        return srcImg;
    }

    else if (srcImg.type() == CV_8UC3) {
        return tintMagentaTemplate<cv::Vec3b, uchar>(srcImg, changeFactor);
    }

    else if (srcImg.type() == CV_16UC3) {
        return tintMagentaTemplate<cv::Vec3w, ushort>(srcImg, changeFactor);
    } else {
        std::cerr << "Error in TintMagenta: unsupported image type\n";
        return cv::Mat();
    }
}