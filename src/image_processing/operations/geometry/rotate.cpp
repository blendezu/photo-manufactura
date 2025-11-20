
#include "rotate.h"

#include <opencv2/core/hal/interface.h>

#include <opencv2/core/mat.hpp>
#include <stdexcept>

cv::Mat Rotate::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        throw std::invalid_argument("Rotate: Input image is empty\n");
    }

    switch (srcImg.type()) {
        case CV_8UC1:  // 8 bit gray image
            return rotateImgTemplate<uchar>(srcImg, angle_deg, roi);
        case CV_8UC3:  // 8 bit RGB image
            return rotateImgTemplate<cv::Vec3b>(srcImg, angle_deg, roi);
        case CV_16UC1:  // 16 bit gray image
            return rotateImgTemplate<uint16_t>(srcImg, angle_deg, roi);
        case CV_16UC3:  // 8 bit RGB image
            return rotateImgTemplate<cv::Vec3w>(srcImg, angle_deg, roi);
        default:
            throw std::invalid_argument("Rotate: unsupported image type\n");
    }
}
