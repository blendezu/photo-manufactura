#ifndef NLM_DENOISE_H
#define NLM_DENOISE_H

#include <algorithm>
#include <opencv2/opencv.hpp>
#include <string>

#include "operation_base.h"

class Denoise : public ImageOperation {
   private:
    int m_strength;  // blending strength 0 -> 100
    cv::Mat m_denoisedImg;
    bool m_reload = true;

   public:
    Denoise(int value) : m_strength(value) {}

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Noise reduction\n";
    }

    std::string getSettings() const override {
        return "strength: " + std::to_string(m_strength);
    }

    void setStrength(int value) {
        m_strength = std::clamp(value, 0, 100);
    }

    int getStrength() {
        return m_strength;
    }

   private:
    // denoise the input image for blending
    void denoiseImg(const cv::Mat& srcImg);
};

#endif