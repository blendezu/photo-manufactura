#pragma once
#include "../core/operation_base.h"

class BinaryFilter : public ImageOperation {
   public:
    std::string getName() const override {
        return "Binary Filter";
    }

    cv::Mat apply(const cv::Mat& scrImg) override {}
};