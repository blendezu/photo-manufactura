#include <Halide.h>

#include <algorithm>
#include <opencv2/opencv.hpp>
#include <string>

#include "operation_base.h"

class AdjustVibrance : public HalideOperation {
   private:
    int m_vibrance;

    // --- Constant Parameters ---
    static float constexpr VIBRANCE_SCALING_FACTOR = 300.0f;
    static float constexpr LOWER_THRESHOLD = 0.3f;
    static float constexpr UPPER_THRESHOLD = 0.6f;

    // --- Halide Runtime Parameters ---
    Halide::Param<float> p_maxRange{"vibrance_maxRange"};
    Halide::Param<float> p_vibranceFactor{"vibranceFactor"};
    Halide::Param<float> p_lowerThreshold{"vibrance_lowerThreshold"};
    Halide::Param<float> p_upperThreshold{"vibrance_upperThreshold"};

   public:
    AdjustVibrance(int value) : m_vibrance(value) {
        p_maxRange.set(255.0f);
        p_lowerThreshold.set(0.0f);
        p_upperThreshold.set(1.0f);
    }

    void prepareParameters(const cv::Mat& srcImg) override;

    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Vibrance";
    }

    std::string getSettings() const override {
        return "vibrance: " + std::to_string(m_vibrance);
    }

    void setVibrance(int value) {
        m_vibrance = std::clamp(value, -100, 100);
    }

    int getVibrance() {
        return m_vibrance;
    }

   private:
    float caculateWeight(float currS) {
        float weight = 0.0f;
        float invRange = 1.0f / (UPPER_THRESHOLD - LOWER_THRESHOLD);

        if (currS < LOWER_THRESHOLD) {
            weight = 1.0f;
        } else if (currS < UPPER_THRESHOLD) {
            float t = (currS - LOWER_THRESHOLD) * invRange;
            weight = 1 - t * t;
        } else {
            weight = 0.0f;
        }
        return weight;
    }
};