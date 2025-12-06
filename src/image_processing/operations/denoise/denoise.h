#ifndef DENOISE_H
#define DENOISE_H

#include <algorithm>
#include <opencv2/opencv.hpp>
#include <string>

#include "operation_base.h"

class Denoise : public ImageOperation {
   private:
    int m_strength;  // blending strength 0 -> 100

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
    cv::Mat denoiseImg(const cv::Mat& srcImg);
};

#endif