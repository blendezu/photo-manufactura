#include <Halide.h>

#include <algorithm>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/saturate.hpp>
#include <string>
#include <vector>

#include "operation_base.h"

class WhiteBalance : public HalideOperation {
   private:
    int m_temp;

    // --- Constant Parameters ---
    static float constexpr WHITE_BALANCE_FACTOR = 200.0f;

    // --- Halide Runtime Parameters ---
    Halide::Param<float> p_changeFactorR{"changeFactorR"};
    Halide::Param<float> p_changeFactorB{"changeFactorB"};

   public:
    WhiteBalance(int value) : m_temp(value) {
        p_changeFactorR.set(0.0f);
        p_changeFactorB.set(0.0f);
    }

    void prepareParameters(const cv::Mat& srcImg) override;

    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Temperature";
    }

    std::string getSettings() const override {
        return "temperature: " + std::to_string(m_temp);
    }

    void setTemperature(int value) {
        m_temp = std::clamp(value, -100, 100);
    }

    int getTemperature() {
        return m_temp;
    }

   private:
    template <typename T>
    cv::Mat whiteBalanceTemplate(const cv::Mat& srcImg, float changeFactorR, float changeFactorB) {
        // Check if the source Image is not Color
        if (srcImg.channels() != 3) {
            std::cerr << "[WhiteBalance] ❌ Error: The Input Image is empty\n";
            return cv::Mat();
        }

        // 1. Create Destination Image
        cv::Mat dstImg(srcImg.size(), srcImg.type());

        // 2. Define LUT size
        int maxRange = std::numeric_limits<T>::max();
        int lutSize = maxRange + 1;

        // 3. Create LUT
        std::vector<T> lutB(lutSize);
        std::vector<T> lutR(lutSize);

        // clang-format off
        // 4. LUTs calculate
        #pragma omp parallel for
        // clang-format on
        for (int i = 0; i < lutSize; i++) {
            float valB = i * changeFactorB;
            float valR = i * changeFactorR;

            lutB[i] = static_cast<T>(std::clamp(valB, 0.0f, static_cast<float>(maxRange)));
            lutR[i] = static_cast<T>(std::clamp(valR, 0.0f, static_cast<float>(maxRange)));
        }

        // 5. Calculate the length of 1D-Array
        int len = srcImg.cols * 3;

        // clang-format off
        // 6. Iteration through the Image and change the values
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < srcImg.rows; y++) {
            // 6.1 Get the pointer of the first pixel of the lines
            // Using __restrict to tell compiler that the Pointers are not aliased
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y);

            // Execute pixel-wise
            for (int i = 0; i < len; i += 3) {
                dstPtr[i] = lutB[srcPtr[i]];  // B

                dstPtr[i + 1] = srcPtr[i + 1];  // G

                dstPtr[i + 2] = lutR[srcPtr[i + 2]];  // R
            }
        }
        return dstImg;
    }
};