#include "clarity.h"

#include <iostream>

#include "../../utils/gaussian.h"

Clarity::Clarity(int value) : m_strength(value) {}

cv::Mat Clarity::apply(const cv::Mat& srcImg) {
    if (srcImg.empty())
        return cv::Mat();
    if (m_strength == 0)
        return srcImg.clone();

    // Clarity = Large Radius Unsharp Mask
    // Use radius proportional to image size or fixed large?
    // Let's use flexible sigma ~ 30.0 for typical Clarity 'punch'.
    float amount = static_cast<float>(m_strength) / CLARITY_SCALING_FACTOR;
    float sigma = 30.0f;

    cv::Mat blurred;
    GaussianFilter::applyCPU(srcImg, blurred, sigma);

    cv::Mat dst;
    // Original + (Original - Blurred) * Amount
    // This boosts local contrast.
    cv::addWeighted(srcImg, 1.0f + amount, blurred, -amount, 0, dst);
    return dst;
}

void Clarity::prepareParameters(const cv::Mat& srcImg) {
    float amount = static_cast<float>(m_strength) / CLARITY_SCALING_FACTOR;
    p_amount.set(amount);
    p_width.set(srcImg.cols);
    p_height.set(srcImg.rows);
}

Halide::Func Clarity::buildGraph(Halide::Func input, Halide::Var x, Halide::Var y, Halide::Var c) {
    float sigma = 30.0f;
    // Blurred version (Large Radius)
    Halide::Func blurred = GaussianFilter::createHalideGraph(input, sigma, p_width, p_height);

    Halide::Func clarity("clarity");
    Halide::Expr valOrig = input(x, y, c);
    Halide::Expr valBlur = blurred(x, y, c);

    // Boost local contrast
    Halide::Expr diff = valOrig - valBlur;
    Halide::Expr res = valOrig + diff * p_amount;

    clarity(x, y, c) = Halide::clamp(res, 0.0f, 1.0f);
    return clarity;
}
