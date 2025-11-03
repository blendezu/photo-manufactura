#include "crop.h"

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
        return srcImg(roi).clone();
    } else {
        std::cerr << "Error in cropImg: the roi area is not valid\n";
        return cv::Mat();
    }
}