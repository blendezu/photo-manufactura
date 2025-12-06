#ifndef GAUSSIAN_H
#define GAUSSIAN_H

#include <opencv2/core/mat.hpp>

class GaussianBlur {
   public:
    static void gaussianBlur(const cv::Mat& srcImg, cv::Mat& dstImg, int kernelSize, double sigma);
};

#endif