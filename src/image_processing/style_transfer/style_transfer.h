#ifndef STYLE_TRANSFER_H
#define STYLE_TRANSFER_H

#include <opencv2/core/mat.hpp>
#include <opencv2/dnn.hpp>
#include <vector>

#include "operation_base.h"

// Enum for available styles
enum class StyleType { Mosaic, Candy, RainPrincess, Udnie, Pointillism };

class StyleTransfer : public ImageOperation {
   private:
    StyleType currentStyle;
    std::string modelPath;

    cv::dnn::Net net;

    void loadModel(StyleType style);

    // for pre and post-processing
    cv::Mat postprocess(const cv::Mat& outputTensor, int rows, int cols);

   public:
    explicit StyleTransfer(StyleType style);  // constructor loads the standard style

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Neural Style Transfer";
    }

    std::string getSettings() const override {
        return "Style: " + std::to_string(static_cast<int>(currentStyle));
    }

    // Setter to change style (Drop out in GUI)
    void setStyle(StyleType style);

    // Set intensity of the style (0.0 - 1.0)
    void setStrength(float strength) {
        this->strength = std::clamp(strength, 0.0f, 1.0f);
    }

   private:
    float strength = 1.0f;
};
#endif  // STYLE_TRANSFER_H