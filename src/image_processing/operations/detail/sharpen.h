#include <algorithm>
#include <string>

#include "operation_base.h"

class Sharpen : public ImageOperation {
   private:
    int sharpen;

   public:
    Sharpen(int value) : sharpen(value) {}

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Sharpen";
    }

    std::string getSettings() const override {
        return "sharpen: " + std::to_string(sharpen);
    }

    void setSharpen(int value) {
        sharpen = std::clamp(value, -100, 100);
    }
};