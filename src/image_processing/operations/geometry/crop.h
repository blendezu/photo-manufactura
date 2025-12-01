#ifndef CROP_H
#define CROP_H

#include <cstring>
#include <opencv2/opencv.hpp>

#include "../core/operation_base.h"

class Crop : public ImageOperation {
   private:
    cv::Rect roi;

   public:
    Crop(cv::Rect roi) : roi(roi) {}

    std::string getName() const override {
        return "Crop";
    }

    /**
     * @brief Crop image to specified rectangular region
     * @param srcImg Input source image no matter which format
     * @return Cropped image region
     * @throws std::invalid_argument if ROI exceeds image boundaries
     */
    cv::Mat apply(const cv::Mat& srcImg) override;

    void setROI(cv::Rect rect) {
        roi = rect;
    }

    cv::Rect getROI() const {
        return roi;
    }

   private:
    template <typename T>
    cv::Mat cropTemplate(const cv::Mat& srcImg) {
        cv::Mat dstImg(roi.height, roi.width, srcImg.type());

        size_t rowBytes = roi.width * sizeof(T);

        // clang-format off
        #pragma omp parallel for
        // clang-format on
        for (int y = roi.y; y < roi.y + roi.height; y++) {
            const T* __restrict srcPtr = srcImg.ptr<T>(y);
            T* __restrict dstPtr = dstImg.ptr<T>(y - roi.y);
            std::memcpy(dstPtr, srcPtr, rowBytes);
        }

        return dstImg;
    }
};

#endif  // CROP_H