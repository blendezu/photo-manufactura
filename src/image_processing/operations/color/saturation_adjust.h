#include <Halide.h>

#include <algorithm>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <string>

#include "operation_base.h"

class AdjustSaturation : public HalideOperation {
   private:
    int m_saturation;

    // --- Constant Parameters ---
    static constexpr float SATURATION_SCALING_FACTOR = 100.0f;

    // --- Halide Runtime Parameter ---
    Halide::Param<float> p_saturationFactor{"saturationFactor"};
    Halide::Param<float> p_maxRange{"sat_maxRange"};

   public:
    AdjustSaturation(int value) : m_saturation(value) {
        p_saturationFactor.set(0.0f);
        p_maxRange.set(255.0f);
    }

    void prepareParameters(const cv::Mat& srcImg) override;

    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Saturation";
    }

    std::string getSettings() const override {
        return "saturation: " + std::to_string(m_saturation);
    }

    void setSaturation(int value) {
        m_saturation = std::clamp(value, -100, 100);
    }

    int getSaturation() {
        return m_saturation;
    }
};