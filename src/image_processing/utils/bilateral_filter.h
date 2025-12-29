#ifndef BILATERAL_FILTER_H
#define BILATERAL_FILTER_H

#include <Halide.h>

#include <opencv2/core.hpp>

class BilateralFilter {
   public:
    /**
     * @brief Apply Bilateral Filter using OpenCV (CPU)
     */
    static void applyCPU(const cv::Mat& src, cv::Mat& dst, int d, double sigmaColor,
                         double sigmaSpace);

    /**
     * @brief Create a Halide graph for Bilateral Filter
     * @param input The input function (assumed normalized 0..1)
     * @param sigmaSpatial Spatial standard deviation (Expr)
     * @param sigmaRange Range (color) standard deviation (Expr)
     * @param maxRadius The window radius (fixed at compile time for loop bounds)
     * @param sourceWidth Width of the input image (Expr)
     * @param sourceHeight Height of the input image (Expr)
     * @return Halide::Func The filtered function
     */
    static Halide::Func createHalideGraph(Halide::Func input, Halide::Expr sigmaSpatial,
                                          Halide::Expr sigmaRange, int maxRadius,
                                          Halide::Expr sourceWidth, Halide::Expr sourceHeight);
};

#endif  // BILATERAL_FILTER_H
