#include "bilateral_filter.h"

#include <cmath>
#include <opencv2/imgproc.hpp>

void BilateralFilter::applyCPU(const cv::Mat& src, cv::Mat& dst, int d, double sigmaColor,
                               double sigmaSpace) {
    if (src.empty())
        return;
    cv::bilateralFilter(src, dst, d, sigmaColor, sigmaSpace);
}

Halide::Func BilateralFilter::createHalideGraph(Halide::Func input, Halide::Expr sigmaSpatial,
                                                Halide::Expr sigmaRange, int maxRadius,
                                                Halide::Expr sourceWidth,
                                                Halide::Expr sourceHeight) {
    // Define the window size based on provided max radius
    int radius = maxRadius;

    Halide::Var x("x"), y("y"), c("c");

    // Manual Clamp for safe access
    // We cannot use repeat_edge easily without implicit bounds on Func,
    // so we clamp coordinates manually.
    Halide::Func clamped("bilateral_clamped");
    clamped(x, y, c) =
        input(Halide::clamp(x, 0, sourceWidth - 1), Halide::clamp(y, 0, sourceHeight - 1), c);

    // Reduction domain: window around (0,0) -> (-radius .. radius)
    Halide::RDom r(-radius, 2 * radius + 1, -radius, 2 * radius + 1, "bilateral_win");

    // Compute center pixel value
    Halide::Expr valCenter = clamped(x, y, c);

    // Compute neighbor pixel value
    Halide::Expr valNeighbor = clamped(x + r.x, y + r.y, c);

    // Spatial weight: Gaussian of distance
    // exp( - (dx^2 + dy^2) / (2 * sigma_s^2) )
    Halide::Expr spatialDistSq = Halide::cast<float>(r.x * r.x + r.y * r.y);
    Halide::Expr weightSpatial = Halide::exp(-spatialDistSq / (2.0f * sigmaSpatial * sigmaSpatial));

    // Range weight: Gaussian of intensity difference
    // exp( - (I_n - I_c)^2 / (2 * sigma_r^2) )
    Halide::Expr rangeDistSq = (valNeighbor - valCenter) * (valNeighbor - valCenter);
    Halide::Expr weightRange = Halide::exp(-rangeDistSq / (2.0f * sigmaRange * sigmaRange));

    // Total weight
    Halide::Expr weight = weightSpatial * weightRange;

    // Weighted Sums
    Halide::Func biFilter("bilateral_filter_sum");
    // We need to sum separately for each channel?
    // Yes, usually bilateral filter is applied per channel OR using luminance for weight.
    // If per channel:
    // sum(val * w) / sum(w)

    Halide::Func numerator("bilateral_num"), denominator("bilateral_denom");
    numerator(x, y, c) = 0.0f;
    denominator(x, y, c) = 0.0f;

    numerator(x, y, c) += valNeighbor * weight;
    denominator(x, y, c) += weight;

    Halide::Func output("bilateral_result");
    output(x, y, c) = numerator(x, y, c) / denominator(x, y, c);

    return output;
}
