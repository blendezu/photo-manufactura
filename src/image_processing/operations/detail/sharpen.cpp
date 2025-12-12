#include "sharpen.h"

#include <iostream>

#include "../../utils/gaussian.h"

Sharpen::Sharpen(int value) : m_strength(value) {}

cv::Mat Sharpen::apply(const cv::Mat& srcImg) {
    if (srcImg.empty())
        return cv::Mat();
    if (m_strength == 0)
        return srcImg.clone();

    // Unsharp mask: Original + (Original - Blurred) * Amount
    float amount = static_cast<float>(m_strength) / 50.0f;  // Scale 100 -> 2.0
    float sigma = 1.0f;                                     // Typical small radius for Sharpen

    cv::Mat blurred;
    GaussianFilter::applyCPU(srcImg, blurred, sigma);

    cv::Mat dst;
    cv::addWeighted(srcImg, 1.0f + amount, blurred, -amount, 0, dst);
    return dst;
}

void Sharpen::prepareParameters(const cv::Mat& srcImg) {
    float amount = static_cast<float>(m_strength) / 50.0f;
    p_amount.set(amount);
    p_width.set(srcImg.cols);
    p_height.set(srcImg.rows);
}

Halide::Func Sharpen::buildGraph(Halide::Func input, Halide::Var x, Halide::Var y, Halide::Var c) {
    float sigma = 1.0f;  // Match CPU
    // Blurred version
    Halide::Func blurred = GaussianFilter::createHalideGraph(input, sigma, p_width, p_height);

    // Original + (Original - Blurred) * Amount
    // = Original * (1 + Amount) - Blurred * Amount

    Halide::Func sharpened("sharpened");
    Halide::Expr valOrig = input(x, y, c);
    Halide::Expr valBlur = blurred(x, y, c);

    // Ensure we are working in float 0..1 range (assumed by pipeline)
    Halide::Expr diff = valOrig - valBlur;
    Halide::Expr res = valOrig + diff * p_amount;

    sharpened(x, y, c) = Halide::clamp(res, 0.0f, 1.0f);
    return sharpened;
}