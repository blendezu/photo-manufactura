#include "denoise.h"

#include <iostream>
#include <opencv2/core.hpp>

#include "../../utils/bilateral_filter.h"

// HalideOperation constructor is called in initializer list
Denoise::Denoise(int value) : m_strength(value) {}

cv::Mat Denoise::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "❌ Error in Denoise: empty input image\n";
        return cv::Mat();
    }

    // Original Logic restored:
    // Strong Bilateral Filter + Blending based on strength.
    int d = 9;
    double sigmaColor = 75.0;
    double sigmaSpace = 75.0;

    cv::Mat denoisedImg;
    BilateralFilter::applyCPU(srcImg, denoisedImg, d, sigmaColor, sigmaSpace);

    double alpha = static_cast<double>(m_strength) / 100.0;
    cv::Mat dstImg;
    cv::addWeighted(srcImg, 1.0 - alpha, denoisedImg, alpha, 0.0, dstImg);
    return dstImg;
}

void Denoise::prepareParameters(const cv::Mat& srcImg) {
    // Fixed "Strong" parameters for Halide to match CPU
    float sigS = 10.0f;  // Approx for d=9 (radius 4) or large space sigma
    // User had sigmaSpace=75. radius 4 means we cover full window.
    // Let's use a reasonable sigma for the window size.

    float sigR = 75.0f / 255.0f;  // ~0.29

    float alpha = static_cast<float>(m_strength) / 100.0f;

    p_sigmaSpatial.set(sigS);
    p_sigmaRange.set(sigR);
    p_blend.set(alpha);
    p_width.set(srcImg.cols);
    p_height.set(srcImg.rows);
}

Halide::Func Denoise::buildGraph(Halide::Func input, Halide::Var x, Halide::Var y, Halide::Var c) {
    // 1. Compute Strong Denoised version
    // Radius 4 (d=9)
    Halide::Func denoised = BilateralFilter::createHalideGraph(input, p_sigmaSpatial, p_sigmaRange,
                                                               4, p_width, p_height);

    // 2. Blend with Original
    // Lerp: input * (1 - blend) + denoised * blend
    Halide::Func output("denoise_blend");
    Halide::Expr valOrig = input(x, y, c);
    Halide::Expr valDenoised = denoised(x, y, c);

    output(x, y, c) = valOrig * (1.0f - p_blend) + valDenoised * p_blend;

    return output;
}