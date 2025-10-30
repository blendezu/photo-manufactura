#ifndef RAW_PROCESSING_H
#define RAW_PROCESSING_H

#include <opencv2/core.hpp>
#include <string>

class RawProcessing {
   public:
    RawProcessing() = default;
    ~RawProcessing() = default;

    cv::Mat getRawImg(const std::string& raw_path);
};

#endif  // RAW_PROCESSING_H