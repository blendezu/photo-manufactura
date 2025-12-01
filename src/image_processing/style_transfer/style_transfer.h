#ifndef STYLE_TRANSFER_H
#define STYLE_TRANSFER_H

#include <onnxruntime_cxx_api.h>

#include <opencv2/core/mat.hpp>
#include <vector>

#include "operation_base.h"

// Enum for available styles
enum class StyleType { Mosaic, Candy, RainPrincess, Udnie, Pointillism };

class StyleTransfer : public ImageOperation {
   private:
    StyleType currentStyle;
    std::string modelPath;

    Ort::Env env;
    Ort::SessionOptions sessionOptions;
    std::unique_ptr<Ort::Session> session;

    void loadModel(StyleType style);

    // for pre and post-processing
    cv::Mat postprocess(const std::vector<float>& floatArray, int rows, int cols);

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
};
#endif  // STYLE_TRANSFER_H