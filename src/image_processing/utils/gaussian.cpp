#include "gaussian.h"

#include <cmath>
#include <opencv2/imgproc.hpp>
#include <vector>

void GaussianFilter::applyCPU(const cv::Mat& src, cv::Mat& dst, float sigma) {
    int ksize = static_cast<int>(std::ceil(sigma * 3.0f)) * 2 + 1;
    cv::GaussianBlur(src, dst, cv::Size(ksize, ksize), sigma, sigma);
}

Halide::Func GaussianFilter::createHalideGraph(Halide::Func input, float sigma,
                                               Halide::Expr sourceWidth,
                                               Halide::Expr sourceHeight) {
    if (sigma <= 0.1f)
        return input;

    Halide::Var x("x"), y("y"), c("c");

    // Create a 1D kernel
    int radius = static_cast<int>(std::ceil(sigma * 3.0f));
    int diameter = 2 * radius + 1;
    std::vector<float> kernel(diameter);
    float sum = 0.0f;
    for (int i = 0; i < diameter; i++) {
        int x_k = i - radius;
        float val = std::exp(-(x_k * x_k) / (2.0f * sigma * sigma));
        kernel[i] = val;
        sum += val;
    }
    // Normalize
    for (int i = 0; i < diameter; i++) {
        kernel[i] /= sum;
    }

    // Manual Clamp for safe access
    Halide::Func clampedInput("gaussian_clamped");
    clampedInput(x, y, c) =
        input(Halide::clamp(x, 0, sourceWidth - 1), Halide::clamp(y, 0, sourceHeight - 1), c);

    // Since kernel is static for a given sigma call, we can embed it into the code if constant,
    // but here sigma varies at runtime in the broader sense, but usually fixed for graph
    // construction if passed as float. If passed as Param, we'd need a different approach
    // (Buffer<float>). For now, assuming sigma is built-time constant for the graph generation or
    // re-generated. The instructions implied mapped from strength, so likely known at rebuild time.

    // Horizontal pass
    Halide::Func blur_x("blur_x");
    Halide::Expr val_x = 0.0f;
    for (int i = 0; i < diameter; i++) {
        int offset = i - radius;
        val_x += clampedInput(x + offset, y, c) * kernel[i];
    }
    blur_x(x, y, c) = val_x;

    // Vertical pass
    Halide::Func blur_y("blur_y");
    Halide::Expr val_y = 0.0f;
    for (int i = 0; i < diameter; i++) {
        int offset = i - radius;
        val_y += blur_x(x, y + offset, c) * kernel[i];
    }
    blur_y(x, y, c) = val_y;

    return blur_y;  // Boundary checks are handled by clamping input and intermediate access logic
                    // if needed, but blur_y accesses blur_x which is defined over all space based
                    // on clampedInput.
    // However, blur_x accesses clampedInput(x+offset), which is safe.
    // blur_y accesses blur_x(x, y+offset).
    // blur_x is defined for all x,y because clampedInput is defined for all x,y.
    // So this is safe.
}
