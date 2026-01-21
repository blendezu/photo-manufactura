#ifndef CROP_H
#define CROP_H

#include <Halide.h>

#include <cstring>
#include <opencv2/opencv.hpp>

#include "../core/operation_base.h"

class Crop : public HalideOperation {
   private:
    cv::Rect m_roi;

    // --- Halide Runtime Parameters ---
    Halide::Param<int> p_x_offset{"crop_x_offset"};
    Halide::Param<int> p_y_offset{"crop_y_offset"};

   public:
    Crop(cv::Rect roi) : m_roi(roi) {
        p_x_offset.set(roi.x);
        p_y_offset.set(roi.y);
    }

    std::string getName() const override {
        return "Crop";
    }

    // --- Halide Implementations ---
    bool supportsHalide() const override {
        return true;
    }

    bool requiresFreshStats() const override {
        return false;
    }

    void prepareParameters(const cv::Mat& srcImg) override;

    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    void getOutputDimensions([[maybe_unused]] int srcWidth, [[maybe_unused]] int srcHeight,
                             int& dstWidth, int& dstHeight) const override {
        dstWidth = m_roi.width;
        dstHeight = m_roi.height;
    }

    /**
     * @brief Crop image to specified rectangular region
     * @param srcImg Input source image no matter which format
     * @return Cropped image region
     * @throws std::invalid_argument if ROI exceeds image boundaries
     */
    cv::Mat apply(const cv::Mat& srcImg) override;

    void setROI(cv::Rect rect) {
        m_roi = rect;
    }

    cv::Rect getROI() const {
        return m_roi;
    }

   private:
    template <typename T>
    cv::Mat cropTemplate(const cv::Mat& srcImg) {
        // 1. Create the Destination Image with the new size
        cv::Mat dstImg(m_roi.height, m_roi.width, srcImg.type());

        // 2. The length of 1D Aray
        size_t rowBytes = m_roi.width * sizeof(T);

        // clang-format off
        // 3. Iteration through the Image using OpenMP for Parallelisim
        #pragma omp parallel for
        // clang-format on
        for (int y = m_roi.y; y < m_roi.y + m_roi.height; y++) {
            // 3.1 Get the pointer of first pixel of the line
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y - m_roi.y);

            // 3.2 Copy
            std::memcpy(dstPtr, srcPtr, rowBytes);
        }

        return dstImg;
    }
};

#endif  // CROP_H