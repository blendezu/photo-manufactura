#ifndef AI_DENOISE_H
#define AI_DENOISE_H

#include <onnxruntime_cxx_api.h>  // WICHTIG: Wieder hinzufügen

#include <memory>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

#include "operation_base.h"

class AiDenoise : public ImageOperation {
   private:
    // ONNX Runtime Ressourcen
    Ort::Env env;
    Ort::SessionOptions sessionOptions;
    std::unique_ptr<Ort::Session> session;

    int strength;  // 0 bis 100

    void loadModel();

    // Hilfsfunktionen
    cv::Mat preprocess(const cv::Mat& srcImg);
    cv::Mat postprocess(const std::vector<float>& outputTensorData, int rows, int cols,
                        const cv::Size& targetSize);

   public:
    explicit AiDenoise(int strengthValue);

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "AI Denoise";
    }

    std::string getSettings() const override {
        return "strength: " + std::to_string(strength);
    }

    void setStrength(int value);
};

#endif