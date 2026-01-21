#include "tint_magenta.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <cstdint>
#include <opencv2/core/mat.hpp>

#include "../../core/halide_build_graph.h"

void TintMagenta::prepareParameters(const cv::Mat& srcImg) {
    // --- Path A. Gray Image ---
    if (srcImg.channels() == 1) {
        std::cerr << "[TintMagenta] ⚠️ Warning: Doesn't support Gray Image\n";
        return;
    }

    // --- Path B. Color Image ---
    // 1. Calculate the changeFactor and Determine maxRange based on the Bit Depth
    float changeFactor = 1 - m_tint / TINT_SCALING_FACTOR;

    // 2. Assign
    p_timaFactor.set(changeFactor);
    // p_maxRange.set(maxRange);
}

Halide::Func TintMagenta::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                                     Halide::Var c) {
    (void)x;
    (void)y;
    (void)c;
    // --- Path A. Gray Image ---
    if (srcImg.dimensions() == 2) {
        return srcImg;
    }

    // --- Path B. Color Image ---
    // Use Shared Logic
    Halide::Func dstImg = HalideBuildGraph::apply_tint(srcImg, p_timaFactor);

    return dstImg;
}

cv::Mat TintMagenta::apply(const cv::Mat& srcImg) {
    // Check if the Input Image is empty
    if (srcImg.empty()) {
        std::cerr << "Error in TintMagenta: empty input image\n";
        return cv::Mat();
    }

    // Calculate the changeFactor
    float changeFactor = 1 - m_tint / TINT_SCALING_FACTOR;

    // --- Path A. Gray Image ---
    if (srcImg.type() == CV_8UC1 || srcImg.type() == CV_16UC1) {
        std::cerr << "[TintMagenta] ⚠️ Warning: Doesn't support Gray Image\n";
        return srcImg;
    }

    // --- Path B. 8-bit Color Image ---
    else if (srcImg.type() == CV_8UC3) {
        return tintMagentaTemplate<cv::Vec3b, uint8_t>(srcImg, changeFactor);
    }

    // --- Path C. 16-bit Color Image ---
    else if (srcImg.type() == CV_16UC3) {
        return tintMagentaTemplate<cv::Vec3w, uint16_t>(srcImg, changeFactor);
    }

    // --- Path D. Unsupported Image Data Type ---
    else {
        std::cerr << "Error in TintMagenta: unsupported image type\n";
        return cv::Mat();
    }
}