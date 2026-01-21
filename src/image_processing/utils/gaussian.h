#ifndef GAUSSIAN_H
#define GAUSSIAN_H

#include <Halide.h>

#include <opencv2/core.hpp>

class GaussianFilter {
   public:
    /**
     * @brief Apply Gaussian Blur using OpenCV (CPU)
     */
    static void applyCPU(const cv::Mat& src, cv::Mat& dst, float sigma);

    /**
     * @brief Create a Halide graph for Gaussian Blur
     * @param input The input function (assumed to be 3-channel or 1-channel, normalized 0..1 or
     * generic)
     * @param sigma The standard deviation of the Gaussian kernel
     * @param sourceWidth Width of the input image
     * @param sourceHeight Height of the input image
     * @return Halide::Func The blurred function
     */
    static Halide::Func createHalideGraph(Halide::Func input, float sigma, Halide::Expr sourceWidth,
                                          Halide::Expr sourceHeight);
};

#endif  // GAUSSIAN_H