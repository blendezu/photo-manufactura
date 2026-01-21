#include "crop.h"

#include <opencv2/core/hal/interface.h>

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <stdexcept>

// --- Halide Implementations ---
void Crop::prepareParameters([[maybe_unused]] const cv::Mat& srcImg) {
    p_x_offset.set(m_roi.x);
    p_y_offset.set(m_roi.y);
}

Halide::Func Crop::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y, Halide::Var c) {
    Halide::Func dstImg("crop_dst");
    dstImg(x, y, c) = srcImg(x + p_x_offset, y + p_y_offset, c);
    return dstImg;
}

// Check ROI and CPU Implementation
cv::Mat Crop::apply(const cv::Mat& srcImg) {
    // check if the image is empty
    if (srcImg.empty()) {
        throw std::invalid_argument("Crop: Input image is empty\n");
    }

    // check if the roi area is valid
    bool inside = m_roi.x >= 0 && m_roi.y >= 0 && m_roi.x + m_roi.width <= srcImg.cols &&
                  m_roi.y + m_roi.height <= srcImg.rows;

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