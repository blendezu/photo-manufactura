#include <string>

#include "operation_base.h"

class AdjustContrast : public ImageOperation {
   private:
    int contrast;

   public:
    AdjustContrast(int value) : contrast(value) {}

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Contrast";
    }

    std::string getSettings() const override {
        return "contrast: " + std::to_string(contrast);
    }

    void setContrast(int value) {
        contrast = std::clamp(value, -100, 100);
    }

    int getContrast() {
        return contrast;
    }
};