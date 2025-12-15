#ifndef CLARITY_H
#define CLARITY_H

#include <algorithm>
#include <string>

#include "../../core/operation_base.h"

class Clarity : public HalideOperation {
   private:
    int m_strength;  // 0..100

    // Halide Params
    Halide::Param<float> p_amount;
    Halide::Param<int> p_width;
    Halide::Param<int> p_height;

   public:
    static float constexpr CLARITY_SCALING_FACTOR = 100.0f;

    explicit Clarity(int value);

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Clarity";
    }

    std::string getSettings() const override {
        return "clarity: " + std::to_string(m_strength);
    }

    void setClarity(int value) {
        m_strength = std::clamp(value, 0, 100);
    }

    // Halide Interface
    Halide::Func buildGraph(Halide::Func input, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;
    void prepareParameters(const cv::Mat& srcImg) override;
};

#endif  // CLARITY_H