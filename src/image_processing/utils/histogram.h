#ifndef HISTOGRAM_H
#define HISTOGRAM_H
#include <opencv2/opencv.hpp>

class Histogram {
   public:
    /**
     * @brief Calculate image histogram for tonal distribution analysis
     * @param iPut Input image (single or multi-channel)
     * @param bins Number of histogram bins (default: 256)
     * @return Vector of histogram bin counts
     */
    static std::tuple<cv::Mat, cv::Mat> histogramImg(const cv::Mat& scr);
};

#endif  // HISTOGRAM_H