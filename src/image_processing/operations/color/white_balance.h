#include <opencv2/core/mat.hpp>
#include <opencv2/core/saturate.hpp>
#include <string>

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
    template <typename V, typename T>
    cv::Mat whiteBalanceTemplate(const cv::Mat& srcImg, float changeFactorR, float changeFactorB) {
        if (srcImg.channels() != 3) {
            std::cerr << "Error in WhiteBalance: empty input image\n";
            return cv::Mat();
        }

        cv::Mat dstImg(srcImg.size(), srcImg.type());  // output image

        for (int y = 0; y < srcImg.rows; y++) {
            const V* srcPtr = srcImg.ptr<V>(y);
            V* dstPtr = dstImg.ptr<V>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                T B = srcPtr[x][0];
                T G = srcPtr[x][1];
                T R = srcPtr[x][2];

                T newB = cv::saturate_cast<T>(B * changeFactorB);
                T newR = cv::saturate_cast<T>(R * changeFactorR);

                dstPtr[x] = V(newB, G, newR);
            }
        }
        return dstImg;
    }
};