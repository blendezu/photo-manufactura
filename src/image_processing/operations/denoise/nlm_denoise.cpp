#include "nlm_denoise.h"

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

void Denoise::denoiseImg(const cv::Mat& srcImg) {
    float h = 7;  // Filter strength: 3-10
    int templateWindowSize = 7;
    int searchWindowSize = 21;

    cv::fastNlMeansDenoisingColored(srcImg, m_denoisedImg, h, h, templateWindowSize,
                                    searchWindowSize);
    std::cout << "denoiseImg\n";
}

cv::Mat Denoise::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "❌ Error in Denoise: empty input image\n";
        return cv::Mat();
    }

    if (m_reload) {
        denoiseImg(srcImg);
        m_reload = false;
    }

    double alpha = static_cast<double>(m_strength) / 100;
    cv::Mat dstImg(srcImg.size(), srcImg.type());
    cv::addWeighted(srcImg, 1 - alpha, m_denoisedImg, alpha, 0.0, dstImg);
    std::cout << "before return\n";
    return dstImg;
}