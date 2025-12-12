#include "white_balance.h"

#include <Halide.h>
#include <opencv2/core/hal/interface.h>

#include <opencv2/core/mat.hpp>

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
    // --- Path A. Gray Image ---
    if (srcImg.dimensions() == 2) {
        return srcImg;
    }

    // --- Path B. Color Image ---
    // 1. Extract BGR Values and cast to float
    Halide::Expr B = Halide::cast<float>(srcImg(x, y, 0));
    Halide::Expr G = Halide::cast<float>(srcImg(x, y, 1));
    Halide::Expr R = Halide::cast<float>(srcImg(x, y, 2));

    // 2. Calculate new R and B Values
    Halide::Expr newR = R * p_changeFactorR;
    Halide::Expr newB = B * p_changeFactorB;

    // 3. Channel Selection
    Halide::Expr val = Halide::select(c == 0, newB, Halide::select(c == 1, G, newR));

    // 4. Assign new Values to Destination Image
    Halide::Func dstImg("white_balance_color_image");
    dstImg(x, y, c) = val;

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