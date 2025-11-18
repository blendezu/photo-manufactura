#include <algorithm>
#include <opencv2/opencv.hpp>
#include <string>

#include "operation_base.h"

class AdjustSaturation : public ImageOperation {
   private:
    int saturation;

   public:
    AdjustSaturation(int value) : saturation(value) {}

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Saturation";
    }

    std::string getSettings() const override {
        return "saturation: " + std::to_string(saturation);
    }

    void setSaturation(int value) {
        saturation = std::clamp(value, -100, 100);
    }

    int getSaturation() {
        return saturation;
    }
};