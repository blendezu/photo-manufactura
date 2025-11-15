#include "image_resize.h"

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

cv::Mat ResizeImage::apply(const cv::Mat& srcImg) {
    // std::cout << "🤙Start resizing ...\n";
    if (srcImg.empty()) {
        std::cerr << "Error in ResizeImage: empty input image\n";
        return cv::Mat();
    }

    if (srcImg.type() == CV_8UC1) {
        return resizeGrayImgTemplate<uchar>(srcImg);
    } else if (srcImg.type() == CV_16UC1) {
        return resizeGrayImgTemplate<ushort>(srcImg);
    } else if (srcImg.type() == CV_8UC3) {
        return resizeBGRImgTemplate<cv::Vec3b>(srcImg);
    } else if (srcImg.type() == CV_16UC3) {
        return resizeBGRImgTemplate<cv::Vec3w>(srcImg);
    } else {
        std::cerr << "Error in ResizeImage: unsupported image type\n";
        return cv::Mat();
    }
}
