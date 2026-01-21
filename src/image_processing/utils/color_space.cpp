#include "color_space.h"

cv::Mat ColorSpace::convertBGR2HSL(const cv::Mat& bgrImg) {
    if (bgrImg.type() == CV_8UC3) {
        return convertBGR2HSLTemplate<cv::Vec3b>(bgrImg);
    } else if (bgrImg.type() == CV_16UC3) {
        return convertBGR2HSLTemplate<cv::Vec3w>(bgrImg);
    } else {
        std::cerr << "Error in converBGR2HSL: unsupported input data type\n";
        return cv::Mat();
    }
}

cv::Mat ColorSpace::convertHSL2BGR(const cv::Mat& hslImg, int bitDepth) {
    // check if the input image is empty
    if (hslImg.empty()) {
        std::cerr << "Error in convertHSL2BGR: the input image is empty\n";
        return cv::Mat();
    }
    if (hslImg.type() != CV_32FC3) {
        std::cerr << "Error in convertHSL2BGR: supported only float image\n";
        return cv::Mat();
    }
    if (bitDepth == 8) {
        return convertHSL2BGRTemplate<CV_8UC3, cv::Vec3b>(hslImg, bitDepth);
    } else if (bitDepth == 16) {
        return convertHSL2BGRTemplate<CV_16UC3, cv::Vec3w>(hslImg, bitDepth);
    } else {
        std::cerr << "Error in convertHSL2BGR: unsupported bit depth (only 8 oder 16)\n";
        return cv::Mat();
    }
}