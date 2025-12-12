#include "../../core/operation_base.h"

class Sharpen : public HalideOperation {
   private:
    int m_strength;  // 0..100

    // Halide Params
    Halide::Param<float> p_amount;
    Halide::Param<int> p_width;
    Halide::Param<int> p_height;

   public:
    Sharpen(int value);

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Sharpen";
    }

    std::string getSettings() const override {
        return "strength: " + std::to_string(m_strength);
    }

    void setStrength(int value) {
        m_strength = std::clamp(value, 0, 100);
    }

    // Halide Interface
    Halide::Func buildGraph(Halide::Func input, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;
    void prepareParameters(const cv::Mat& srcImg) override;
};