#include <algorithm>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/saturate.hpp>
#include <string>
#include <vector>

#include "operation_base.h"

class WhiteBalance : public ImageOperation {
   private:
    int temp;

   public:
    WhiteBalance(int value) : temp(value) {}

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Temperature";
    }

    std::string getSettings() const override {
        return "temperature: " + std::to_string(temp);
    }

    void setTemperature(int value) {
        temp = std::clamp(value, -100, 100);
    }

    int getTemperature() {
        return temp;
    }

   private:
    template <typename T>
    cv::Mat whiteBalanceTemplate(const cv::Mat& srcImg, float changeFactorR, float changeFactorB) {
        if (srcImg.channels() != 3) {
            std::cerr << "Error in WhiteBalance: empty input image\n";
            return cv::Mat();
        }

        cv::Mat dstImg(srcImg.size(), srcImg.type());

        // LUT size
        int maxRange = std::numeric_limits<T>::max();
        int lutSize = maxRange + 1;

        // create LUT
        std::vector<T> lutB(lutSize);
        std::vector<T> lutR(lutSize);

// LUT calculate
#pragma omp parallel for
        for (int i = 0; i < lutSize; i++) {
            float valB = i * changeFactorB;
            float valR = i * changeFactorR;

            lutB[i] = static_cast<T>(std::clamp(valB, 0.0f, static_cast<float>(maxRange)));
            lutR[i] = static_cast<T>(std::clamp(valR, 0.0f, static_cast<float>(maxRange)));
        }

        int len = srcImg.cols * 3;

#pragma omp parallel for
        for (int y = 0; y < srcImg.rows; y++) {
            const T* srcPtr = srcImg.ptr<T>(y);
            T* dstPtr = dstImg.ptr<T>(y);

            for (int i = 0; i < len; i += 3) {
                dstPtr[i] = lutB[srcPtr[i]];  // B

                dstPtr[i + 1] = srcPtr[i + 1];  // G

                dstPtr[i + 2] = lutR[srcPtr[i + 2]];  // R
            }
        }
        return dstImg;
    }
};