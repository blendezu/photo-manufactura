#pragma once

#include <opencv2/opencv.hpp>

class RawProcessing {
   public:
    RawProcessing() = default;
    ~RawProcessing() = default;

    cv::Mat getRawImg(const std::string& raw_path);
};
