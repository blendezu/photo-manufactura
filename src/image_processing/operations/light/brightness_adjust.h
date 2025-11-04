#ifndef BRIGHTNESS_ADJUST_H
#define BRIGHTNESS_ADJUST_H

#include <string>

#include "../core/operation_base.h"

class BrightnessAdjust : public ImageOperation {
   private:
    int brightness;  // -100 --> 100

   public:
    BrightnessAdjust(int value) : brightness(value) {}

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Brightness";
    }

    std::string getSettings() const override {
        return "brightness: " + std::to_string(brightness);
    }

    void setBrightness(int value) {
        brightness = std::clamp(value, -100, 100);
    }

    int getBrightness() {
        return brightness;
    }
};

#endif