#pragma once
#include "../core/operation_base.h"

class Flip : public ImageOperation {
   private:
    int flipDir;

   public:
    Flip(int flipDirdirection) : flipDir(flipDirdirection) {}

    cv::Mat apply(const cv::Mat& scrImg) override;

    std::string getName() const override {
        return "Flip";
    }

    std::string getSettings() const override {
        return "direction: " + std::to_string(flipDir);
    }

    void setDirection(int direction) {
        flipDir = direction;
    }

    int getDirection() {
        return flipDir;
    }

   private:
    /**
     * @brief Flip image along horizontal or vertical axis
     * @param srcImg Input source image
     * @param flipCode 0 for vertical (↕) flipping, 1 for horizontal (↔)
     * @return Flipped image
     */
    template <typename T>
    static cv::Mat flipImgTemplate(const cv::Mat& srcImg, int flipCode) {
        // check if the input image is empty
        if (srcImg.empty()) {
            std::cerr << "Error in flipImgTemplate: the input image is empty\n";
            return cv::Mat();
        }
        const int imgW = srcImg.cols;
        const int imgH = srcImg.rows;
        // const int channels = srcImg.channels();

        cv::Mat flippedImg(srcImg.size(), srcImg.type());

        const bool horizontal = (flipCode == 1);

        // --- Horizontal flip (left <-> right) ---
        if (horizontal) {
            // clang-format off
            #pragma omp parallel for
            // clang-format on
            for (int y = 0; y < imgH; y++) {
                const T* srcPtr = srcImg.ptr<T>(y);
                T* flipPtr = flippedImg.ptr<T>(y);

                for (int x = 0; x < imgW; x++) {
                    flipPtr[imgW - 1 - x] = srcPtr[x];
                }
            }
        }
        // --- Vertical flip (top ↕ bottom) ---
        else {
            // clang-format off
            #pragma omp parallel for
            //clang-format on
            for (int y = 0; y < imgH; y++) {
                const T* srcPtr = srcImg.ptr<T>(y);
                T* flipPtr = flippedImg.ptr<T>(imgH - 1 - y);

                // copy srcPtr to flipPtr with memory copy
                std::memcpy(flipPtr, srcPtr, imgW * sizeof(T));
            }
        }
        return flippedImg;
    }
};