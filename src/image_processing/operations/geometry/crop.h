#ifndef CROP_H
#define CROP_H

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
};

#endif  // CROP_H