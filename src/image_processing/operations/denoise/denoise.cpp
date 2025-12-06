#include "denoise.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

cv::Mat Denoise::denoiseImg(const cv::Mat& srcImg) {
    // float h = 7;  // Filter strength: 3-10
    // int templateWindowSize = 7;
    // int searchWindowSize = 21;

    // cv::fastNlMeansDenoisingColored(srcImg, m_denoisedImg, h, h, templateWindowSize,
    //                                 searchWindowSize);
    // std::cout << "denoiseImg\n";

    int d = 9;

    double sigmaColor = 75.0;

    double sigmaSpace = 75.0;
    cv::Mat denoisedImg;
    cv::bilateralFilter(srcImg, denoisedImg, d, sigmaColor, sigmaSpace);
    return denoisedImg;
}

cv::Mat Denoise::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "❌ Error in Denoise: empty input image\n";
        return cv::Mat();
    }

    cv::Mat denoisedImg = denoiseImg(srcImg);

    double alpha = static_cast<double>(m_strength) / 100;
    cv::Mat dstImg(srcImg.size(), srcImg.type());
    cv::addWeighted(srcImg, 1 - alpha, denoisedImg, alpha, 0.0, dstImg);
    std::cout << "before return\n";
    return dstImg;
}