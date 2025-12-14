#include "white_balance.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <opencv2/core/mat.hpp>

#include "../../core/halide_build_graph.h"

void WhiteBalance::prepareParameters(const cv::Mat& srcImg) {
    // --- Path A. Gray Image ---
    if (srcImg.channels() == 1) {
        return;
    }

    // --- Path B. Color Image ---
    float changeFactorR = 1 + m_temp / WHITE_BALANCE_FACTOR;
    float changeFactorB = 1 - m_temp / WHITE_BALANCE_FACTOR;

    p_changeFactorR.set(changeFactorR);
    p_changeFactorB.set(changeFactorB);
}

Halide::Func WhiteBalance::buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
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
    Halide::Func dstImg =
        HalideBuildGraph::apply_white_balance(srcImg, p_changeFactorR, p_changeFactorB);

    return dstImg;
}

cv::Mat WhiteBalance::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in WhiteBalance: empty input image\n";
        cv::Mat();
    }
    float changeFactorR = 1 + m_temp / WHITE_BALANCE_FACTOR;
    float changeFactorB = 1 - m_temp / WHITE_BALANCE_FACTOR;

    if (srcImg.type() == CV_8UC1 || srcImg.type() == CV_16UC1) {
        return srcImg;
    }

    else if (srcImg.type() == CV_8UC3) {
        return whiteBalanceTemplate<uint8_t>(srcImg, changeFactorR, changeFactorB);
    }

    else if (srcImg.type() == CV_16UC3) {
        return whiteBalanceTemplate<uint16_t>(srcImg, changeFactorR, changeFactorB);
    } else {
        std::cerr << "Error in WhiteBalance: unsupported image type\n";
        return cv::Mat();
    }
}