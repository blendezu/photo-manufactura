#ifndef OPERATION_BASE_H
#define OPERATION_BASE_H

#include <opencv2/opencv.hpp>
#include <string>

class ImageOperation {
   public:
    virtual ~ImageOperation() = default;

    // apply this function on the image
    virtual cv::Mat apply(const cv::Mat& srcImg) = 0;

    // name for the function on the GUI
    virtual std::string getName() const = 0;

    // settings (for save/load)
    virtual std::string getSettings() const {
        return "";
    }
};

#endif