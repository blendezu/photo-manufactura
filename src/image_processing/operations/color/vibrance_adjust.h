#include <algorithm>
#include <opencv2/opencv.hpp>
#include <string>

#include "color_space.h"
#include "operation_base.h"

class AdjustVibrance : public ImageOperation {
   private:
    int vibrance;

   public:
    AdjustVibrance(int value) : vibrance(value) {}

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Vibrance";
    }

    std::string getSettings() const override {
        return "vibrance: " + std::to_string(vibrance);
    }

    void setVibrance(int value) {
        vibrance = std::clamp(value, -100, 100);
    }

    int getVibrance() {
        return vibrance;
    }

   private:
    float caculateWeight(float currS) {
        float under = 0.3f;
        float middle = 0.6f;
        float weight = 0.0f;

        if (currS < under) {
            weight = 1.0f;
        } else if (currS < middle) {
            float t = (currS - under) / (middle - under);
            weight = 1 - t * t;
        } else {
            weight = 0.0f;
        }
        return weight;
    }
};