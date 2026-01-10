#ifndef STYLE_TRANSFER_H
#define STYLE_TRANSFER_H

#include <opencv2/core/mat.hpp>
#include <opencv2/dnn.hpp>

#include "operation_base.h"

// Enum for available styles
enum class StyleType { Mosaic, Candy, RainPrincess, Udnie, Pointillism };

// Struct to hold all variation parameters
struct StyleVariationParams {
    int hueVariation = 0;       // -100 to +100 (maps to hue shift)
    int satVariation = 0;       // 0-100 (saturation intensity)
    int contrastVariation = 0;  // 0-100 (contrast intensity)
    int noiseAmount = 0;        // 0-100 (noise intensity)
};

class StyleTransfer : public ImageOperation {
   private:
    StyleType currentStyle;
    std::string modelPath;

    cv::dnn::Net net;

    void loadModel(StyleType style);

    // for pre and post-processing
    cv::Mat postprocess(const cv::Mat& outputTensor, int rows, int cols);

    // Apply variation based on parameters
    cv::Mat applyVariation(const cv::Mat& input);

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

    // Individual variation setters
    void setHueVariation(int value) {
        m_params.hueVariation = std::clamp(value, -100, 100);
    }
    void setSatVariation(int value) {
        m_params.satVariation = std::clamp(value, 0, 100);
    }
    void setContrastVariation(int value) {
        m_params.contrastVariation = std::clamp(value, 0, 100);
    }
    void setNoiseAmount(int value) {
        m_params.noiseAmount = std::clamp(value, 0, 100);
    }

    // Set all parameters at once
    void setVariationParams(const StyleVariationParams& params) {
        m_params = params;
    }

   private:
    StyleVariationParams m_params;
};
#endif  // STYLE_TRANSFER_H