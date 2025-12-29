
#include "rotate.h"

#include <opencv2/core/hal/interface.h>

#include <cmath>
#include <opencv2/core/mat.hpp>
#include <stdexcept>

// --- Halide Implementations ---
void Rotate::prepareParameters(const cv::Mat& srcImg) {
    // 1. Calculate Center
    int cx = srcImg.cols / 2;
    int cy = srcImg.rows / 2;

    // 2. Calculate Rotation Matrix (Inverse Mapping)
    double angle_rad = m_angle_deg * M_PI / 180.0;
    float cosA = static_cast<float>(std::cos(angle_rad));
    float sinA = static_cast<float>(std::sin(angle_rad));

    // 3. Set Parameters
    p_cosA.set(cosA);
    p_sinA.set(sinA);
    p_cx.set(cx);
    p_cy.set(cy);
    p_src_width.set(srcImg.cols);
    p_src_height.set(srcImg.rows);

    if (m_roi.empty()) {
        p_roi_x.set(0);
        p_roi_y.set(0);
    } else {
        p_roi_x.set(m_roi.x);
        p_roi_y.set(m_roi.y);
    }
}

Halide::Func Rotate::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y, Halide::Var c) {
    // 1. Handle Boundaries
    // Halide::Func doesn't support .width()/.height() inside the graph directly if it's not an
    // ImageParam. So we use manual boundary checks with 'select'.

    // Cast inputs to float for interpolation first
    Halide::Func srcFloat("src_float");
    srcFloat(x, y, c) = Halide::cast<float>(srcImg(x, y, c));

    // 2. Coordinate Transformation (Inverse Mapping)
    // Adjust for ROI offset since x,y are relative to the Output Buffer (0,0 is start of ROI)
    Halide::Expr dstX = x + p_roi_x;
    Halide::Expr dstY = y + p_roi_y;

    // Shift to center-relative coordinates (Center of Rotation is Center of Source Image)
    Halide::Expr dx = dstX - p_cx;
    Halide::Expr dy = dstY - p_cy;

    // Rotate (Standard Rotation Matrix for inverse mapping)
    Halide::Expr srcX = dx * p_cosA + dy * p_sinA + p_cx;
    Halide::Expr srcY = -dx * p_sinA + dy * p_cosA + p_cy;

    // 3. Bilinear Interpolation
    // Integer part (floor)
    Halide::Expr x_i = Halide::cast<int>(Halide::floor(srcX));
    Halide::Expr y_i = Halide::cast<int>(Halide::floor(srcY));

    // Fractional part
    Halide::Expr u = srcX - x_i;
    Halide::Expr v = srcY - y_i;

    // Helper: Clamped access with 0 padding
    auto clampedSrc = [&](Halide::Expr px, Halide::Expr py) {
        return Halide::select(px >= 0 && px < p_src_width && py >= 0 && py < p_src_height,
                              srcFloat(px, py, c), 0.0f);
    };

    // Sample 4 neighbors
    Halide::Expr p00 = clampedSrc(x_i, y_i);
    Halide::Expr p10 = clampedSrc(x_i + 1, y_i);
    Halide::Expr p01 = clampedSrc(x_i, y_i + 1);
    Halide::Expr p11 = clampedSrc(x_i + 1, y_i + 1);

    // Interpolate
    Halide::Expr interpVal = (p00 * (1.0f - u) * (1.0f - v)) + (p10 * u * (1.0f - v)) +
                             (p01 * (1.0f - u) * v) + (p11 * u * v);

    // 4. Assign
    Halide::Func dst("rotate_dst");
    dst(x, y, c) = interpVal;

    return dst;
}

cv::Mat Rotate::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        throw std::invalid_argument("Rotate: Input image is empty\n");
    }

    switch (srcImg.type()) {
        case CV_8UC1:  // 8 bit gray image
            return rotateImgTemplate<uint8_t>(srcImg, m_angle_deg, m_roi);
        case CV_8UC3:  // 8 bit RGB image
            return rotateImgTemplate<cv::Vec3b>(srcImg, m_angle_deg, m_roi);
        case CV_16UC1:  // 16 bit gray image
            return rotateImgTemplate<uint16_t>(srcImg, m_angle_deg, m_roi);
        case CV_16UC3:  // 8 bit RGB image
            return rotateImgTemplate<cv::Vec3w>(srcImg, m_angle_deg, m_roi);
        default:
            throw std::invalid_argument("Rotate: unsupported image type\n");
    }
}
