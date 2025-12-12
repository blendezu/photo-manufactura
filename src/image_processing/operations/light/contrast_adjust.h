#pragma once
#include <Halide.h>

#include <string>

#include "operation_base.h"

class AdjustContrast : public HalideOperation {
   private:
    int m_contrast;

    // --- Halide Runtime Parameters ---
    Halide::Param<float> p_maxRange{"p_contrast_maxRange"};
    Halide::Param<float> p_contrastFactor{"contrastFactor"};

    // --- Constant Parameters ---
   public:
    static constexpr float CONTRAST_SCALING_FACTOR = 100.0f;

   private:
   public:
    AdjustContrast(int value) : m_contrast(value) {
        p_maxRange.set(255.0f);
        p_contrastFactor.set(0.0f);
    }

    bool requiresFreshStats() const override {
        return false;
    }

    void prepareParameters(const cv::Mat& srcImg) override;

    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Contrast";
    }

    std::string getSettings() const override {
        return "contrast: " + std::to_string(m_contrast);
    }

    void setContrast(int value) {
        m_contrast = std::clamp(value, -100, 100);
    }

    int getContrast() {
        return m_contrast;
    }

   private:
    template <typename T>
    cv::Mat contrastGrayImgTemplate(const cv::Mat& srcImg, float contrastFactor) {
        // 1. Create Destination Image
        cv::Mat dstImg(srcImg.size(), srcImg.type());

        // 2. Define the half max Range for calculation later
        int halfMaxRange = (srcImg.depth() == CV_8U) ? 128 : 32768;

        // clang-format off
        // 3. Iteration through the image
        #pragma omp parallel for
        // clang-format on

        for (int y = 0; y < srcImg.rows; y++) {
            // 3.1 Get the pointers using __restrict to tell compiller that the pointer are not
            // aliased.
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

            // 3.2 Calculate pixel-wise
            for (int x = 0; x < srcImg.cols; x++) {
                // 3.2.1 Calculate the new value
                int newVal =
                    static_cast<int>((srcPtr[x] - halfMaxRange) * contrastFactor + halfMaxRange);
                dstPtr[x] = cv::saturate_cast<T>(newVal);
            }
        }
        return dstImg;
    }
};