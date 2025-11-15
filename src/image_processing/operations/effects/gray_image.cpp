#include "gray_image.h"

#include <opencv2/core/hal/interface.h>

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

cv::Mat GrayImage::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in GrayImage: empty input image\n";
        return cv::Mat();
    }
    if (srcImg.type() == CV_8UC3) {
        return grayImgTemplate<cv::Vec3b, uchar>(srcImg);
    } else if (srcImg.type() == CV_16UC3) {
        return grayImgTemplate<cv::Vec3w, ushort>(srcImg);
    } else if (srcImg.type() == CV_8UC1 || srcImg.type() == CV_16UC1) {
        return srcImg;
    } else {
        std::cerr << "Error in GrayImage: unsupported image type\n";
        return cv::Mat();
    }
}