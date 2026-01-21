#pragma once
#include <Halide.h>

#include "../core/operation_base.h"

class Flip : public HalideOperation {
   private:
    int m_flipDirrection;

    // --- Halide Runtime Parameters ---
    Halide::Param<int> p_width{"flip_width"};
    Halide::Param<int> p_height{"flip_height"};
    Halide::Param<bool> p_isHorizontal{"isHorizonatal"};

   public:
    Flip(int flipDirdirection) {
        if (flipDirdirection == 0 || flipDirdirection == 1) {
            m_flipDirrection = flipDirdirection;
        } else {
            std::cerr << "[Flip] Warning: Invalid direction " << flipDirdirection
                      << ". Defaulting to 1 (Horizontal).\n";
            m_flipDirrection = 1;
        }

        p_width.set(0);
        p_height.set(0);
        p_isHorizontal.set(false);
    }

    void prepareParameters(const cv::Mat& srcImg) override;

    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    cv::Mat apply(const cv::Mat& scrImg) override;

    std::string getName() const override {
        return "Flip";
    }

    std::string getSettings() const override {
        return "direction: " + std::to_string(m_flipDirrection);
    }

    void setDirection(int direction) {
        if (direction == 0 || direction == 1) {
            m_flipDirrection = direction;
        }
    }

    int getDirection() {
        return m_flipDirrection;
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
        // 1. Check if the input image is empty
        if (srcImg.empty()) {
            std::cerr << "Error in flipImgTemplate: the input image is empty\n";
            return cv::Mat();
        }

        // 2. Create the Destination Image with same Dimension and Type
        const int imgW = srcImg.cols;
        const int imgH = srcImg.rows;
        cv::Mat flippedImg(srcImg.size(), srcImg.type());

        // 3. Check if the Flip Direction is horizontal
        const bool horizontal = (flipCode == 1);

        // --- Path A. Horizontal flip (left <-> right) ---
        if (horizontal) {
            // clang-format off
            // 1. Iteration through the Image using OpenMP for Parallelism
            #pragma omp parallel for
            // clang-format on
            for (int y = 0; y < imgH; y++) {
                // 2. Get the pointers of first pixels each line
                // Using __restrict to tell compiler that the Pointers are not aliased.
                const T* __restrict srcPtr = srcImg.ptr<T>(y);
                T* __restrict flipPtr = flippedImg.ptr<T>(y);

                // 3. Swap the Values
                for (int x = 0; x < imgW; x++) {
                    flipPtr[imgW - 1 - x] = srcPtr[x];
                }
            }
        }
        // --- Path B. Vertical flip (top ↕ bottom) ---
        else {
            // clang-format off
            // 1. Iteration through the Image using OpenMP for Parallelism
            #pragma omp parallel for
            //clang-format on
            for (int y = 0; y < imgH; y++) {
                // 2. Get the pointer of the first pixels
                const T* __restrict srcPtr = srcImg.ptr<T>(y);
                T* __restrict flipPtr = flippedImg.ptr<T>(imgH - 1 - y);

                // 3. Copy srcPtr to flipPtr with memory copy
                std::memcpy(flipPtr, srcPtr, imgW * sizeof(T));
            }
        }
        return flippedImg;
    }
};