#include "crop.h"

#include <opencv2/core/hal/interface.h>

#include <opencv2/core.hpp>
#include <stdexcept>

cv::Mat Crop::apply(const cv::Mat& srcImg) {
    // check if the image is empty
    if (srcImg.empty()) {
        throw std::invalid_argument("Crop: Input image is empty\n");
    }

    // check if the roi area is valid
    bool inside = roi.x >= 0 && roi.y >= 0 && roi.x + roi.width <= srcImg.cols &&
                  roi.y + roi.height <= srcImg.rows;

    if (inside) {
        switch (srcImg.type()) {
            case CV_8UC1:
                return cropTemplate<uchar>(srcImg);

            case CV_8UC3:
                return cropTemplate<cv::Vec3b>(srcImg);

            case CV_16UC1:
                return cropTemplate<uint16_t>(srcImg);

            case CV_16UC3:
                return cropTemplate<cv::Vec3w>(srcImg);

            default:
                std::cerr << "Error in Crop: unsupported image type\n";
                return cv::Mat();
        }

    } else {
        std::cerr << "Error in cropImg: the roi area is not valid\n";
        return cv::Mat();
    }
}