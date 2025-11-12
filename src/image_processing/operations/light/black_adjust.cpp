#include "black_adjust.h"

#include <opencv2/core/hal/interface.h>

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

cv::Mat AdjustBlack::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in AdjustBlack: empty input image\n";
        return cv::Mat();
    }

    float blackFactor = black / 800.0f;
    switch (srcImg.type()) {
        case CV_8UC1:
            return blackGrayImgTemplate<uchar>(srcImg, blackFactor);
        case CV_16UC1:
            return blackGrayImgTemplate<ushort>(srcImg, blackFactor);
        case CV_8UC3:
            return blackRGBImgTemplate<cv::Vec3b>(srcImg, blackFactor);
        case CV_16UC3:
            return blackRGBImgTemplate<cv::Vec3w>(srcImg, blackFactor);
        default:
            std::cerr << "Error in AdjustBlack: unsupported image type\n";
            return cv::Mat();
    }
}