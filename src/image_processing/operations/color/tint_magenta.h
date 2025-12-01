#pragma once
#include <algorithm>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/saturate.hpp>
#include <string>

#include "operation_base.h"

/**
 * @class TintMagenta
 * @brief Applies a subtle magenta tint to color images.
 *
 * This class inherits from ImageOperation and uses a template-based approach
 * to efficiently process both 8-bit and 16-bit 3-channel images.
 *
 * The intensity of the magenta tint can be adjusted with the 'tint' parameter,
 * which is clamped within the range [-100, 100] --> [tint, magenta]
 */
class TintMagenta : public ImageOperation {
   private:
    int tint;

   public:
    TintMagenta(int value) : tint(value) {}

    /**
     * @brief Applies magenta tint to the provided image.
     * @param srcImg Input image (CV_8UC3 or CV_16UC3)
     * @return New image with modified green channel
     */
    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Tint";
    }

    std::string getSettings() const override {
        return "tint: " + std::to_string(tint);
    }

    void setTint(int value) {
        tint = std::clamp(value, -100, 100);
    }

    int getTint() {
        return tint;
    }

   private:
    template <typename V, typename T>
    cv::Mat tintMagentaTemplate(const cv::Mat& srcImg, float changeFactor) {
        cv::Mat dstImg(srcImg.size(), srcImg.type());

#pragma omp parallel for
        for (int y = 0; y < srcImg.rows; y++) {
            const V* srcPtr = srcImg.ptr<V>(y);
            V* dstPtr = dstImg.ptr<V>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                T B = srcPtr[x][0];
                T G = srcPtr[x][1];
                T R = srcPtr[x][2];

                // change only G channel
                T newG = cv::saturate_cast<T>(G * changeFactor);

                dstPtr[x] = V(B, newG, R);
            }
        }
        return dstImg;
    }
};