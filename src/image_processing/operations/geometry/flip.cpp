#include "flip.h"

#include <opencv2/core/hal/interface.h>

#include <stdexcept>

cv::Mat Flip::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        throw std::invalid_argument("Flip: the input image is empty\n");
    }

    switch (srcImg.type()) {
        case CV_8UC1:
            return flipImgTemplate<uchar>(srcImg, flipDir);
        case CV_8UC3:
            return flipImgTemplate<cv::Vec3b>(srcImg, flipDir);
        case CV_16UC1:
            return flipImgTemplate<uint16_t>(srcImg, flipDir);
        case CV_16UC3:
            return flipImgTemplate<cv::Vec3w>(srcImg, flipDir);
        default:
            throw std::invalid_argument("Flip: unsupported data type\n");
    }
}