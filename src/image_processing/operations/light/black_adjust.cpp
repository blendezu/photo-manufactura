#include "black_adjust.h"

#include <opencv2/core/hal/interface.h>

#include <opencv2/opencv.hpp>

cv::Mat AdjustBlack::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in AdjustBlack: empty input image\n";
        return cv::Mat();
    }
    return blackGrayImgTemplate<uchar>(srcImg, black / 800.0f);
}