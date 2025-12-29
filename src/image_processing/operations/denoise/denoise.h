#ifndef DENOISE_H
#define DENOISE_H

#include <algorithm>
#include <opencv2/opencv.hpp>
#include <string>

#include "../../core/operation_base.h"

class Denoise : public HalideOperation {
   private:
    int m_strength;  // blending strength 0 -> 100

    // Halide params
    Halide::Param<float> p_sigmaSpatial;
    Halide::Param<float> p_sigmaRange;
    Halide::Param<float> p_blend;  // Alpha for blending
    Halide::Param<int> p_width;
    Halide::Param<int> p_height;

   public:
    Denoise(int value);

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Noise reduction";
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

    // Halide Interface
    Halide::Func buildGraph(Halide::Func input, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;
    void prepareParameters(const cv::Mat& srcImg) override;
};

#endif