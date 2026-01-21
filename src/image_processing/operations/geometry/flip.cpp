#include "flip.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <stdexcept>

void Flip::prepareParameters(const cv::Mat& srcImg) {
    int width = srcImg.cols;
    int height = srcImg.rows;
    bool isHorizonatal = (m_flipDirrection == 1) ? true : false;

    p_height.set(height);
    p_width.set(width);
    p_isHorizontal.set(isHorizonatal);
}

Halide::Func Flip::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y, Halide::Var c) {
    // 1. Define x if horizontal Flip
    Halide::Expr horizontalX = p_width - 1 - x;

    // 2. Define y if vertical Flip
    Halide::Expr verticalY = p_height - 1 - y;

    // 3. Select newX and newY
    Halide::Expr newX = Halide::select(p_isHorizontal, horizontalX, x);
    Halide::Expr newY = Halide::select(p_isHorizontal, y, verticalY);

    // 4. Swap values
    Halide::Func dstImg("flip_image");
    dstImg(x, y, c) = srcImg(newX, newY, c);

    return dstImg;
}

cv::Mat Flip::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        throw std::invalid_argument("Flip: the input image is empty\n");
    }

    switch (srcImg.type()) {
        case CV_8UC1:
            return flipImgTemplate<uint8_t>(srcImg, m_flipDirrection);
        case CV_8UC3:
            return flipImgTemplate<cv::Vec3b>(srcImg, m_flipDirrection);
        case CV_16UC1:
            return flipImgTemplate<uint16_t>(srcImg, m_flipDirrection);
        case CV_16UC3:
            return flipImgTemplate<cv::Vec3w>(srcImg, m_flipDirrection);
        default:
            throw std::invalid_argument("Flip: unsupported data type\n");
    }
}