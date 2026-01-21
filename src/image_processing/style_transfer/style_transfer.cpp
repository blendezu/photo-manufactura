#include "style_transfer.h"

#include <opencv2/core/hal/interface.h>

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <random>
#include <vector>

StyleTransfer::StyleTransfer(StyleType style) : currentStyle(style) {
    // Load first model
    loadModel(currentStyle);
}

void StyleTransfer::loadModel(StyleType style) {
    currentStyle = style;

    std::string modelName;

    switch (style) {
        case StyleType::Mosaic:
            modelName = "mosaic-9.onnx";
            break;
        case StyleType::Candy:
            modelName = "candy-8.onnx";
            break;
        case StyleType::RainPrincess:
            modelName = "rain-princess-9.onnx";
            break;
        case StyleType::Udnie:
            modelName = "udnie-9.onnx";
            break;
        case StyleType::Pointillism:
            modelName = "pointilism.onnx";
            break;
        default:
            modelName = "mosaic-9.onnx";
            break;
    }

    // List of potential paths to check
    std::vector<std::string> baseDirs = {
        "AI_models/",               // Standard relative path (dev/bin)
        "../Resources/AI_models/",  // macOS Bundle Resources
        "../../../AI_models/",      // Fallback relative to bin if run from bundle
        "../AI_models/"             // Sibling directory
    };

    std::string fullPath;
    bool found = false;

    for (const auto& dir : baseDirs) {
        std::string testPath = dir + modelName;
        FILE* f = fopen(testPath.c_str(), "r");
        if (f) {
            fclose(f);
            fullPath = testPath;
            found = true;
            std::cout << "[StyleTransfer] Found model at: " << fullPath << std::endl;
            break;
        }
    }

    if (!found) {
        fullPath = "AI_models/" + modelName;
        std::cerr << "⚠️ Could not locate model " << modelName << " in any expected path."
                  << std::endl;
    }

    std::cout << "Loading model from " << fullPath << std::endl;

    try {
        net = cv::dnn::readNetFromONNX(fullPath);
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

        if (net.empty()) {
            std::cerr << "❌ Error: Could not load model " << fullPath << std::endl;
        }

    } catch (const cv::Exception& e) {
        std::cerr << "❌ OpenCV DNN Error loading model: " << e.what() << std::endl;
        std::cerr << "Tried path: " << fullPath << std::endl;
    }
}

void StyleTransfer::setStyle(StyleType style) {
    if (currentStyle != style) {
        loadModel(style);
    }
}

cv::Mat StyleTransfer::postprocess(const cv::Mat& outputTensor, int rows, int cols) {
    const float* data = (float*)outputTensor.data;

    std::vector<cv::Mat> channels;
    int channelSize = rows * cols;

    for (int i = 0; i < 3; i++) {
        const float* channelData = data + (i * channelSize);
        cv::Mat channel(rows, cols, CV_32F, const_cast<float*>(channelData));
        channels.push_back(channel);
    }

    cv::Mat mergedImg;
    cv::merge(channels, mergedImg);

    cv::threshold(mergedImg, mergedImg, 255.0, 255.0, cv::THRESH_TRUNC);
    cv::threshold(mergedImg, mergedImg, 0.0, 0.0, cv::THRESH_TOZERO);

    cv::Mat img8;
    mergedImg.convertTo(img8, CV_8U);

    cv::cvtColor(img8, img8, cv::COLOR_RGB2BGR);

    return img8;
}

cv::Mat StyleTransfer::applyVariation(const cv::Mat& input) {
    // Check if any variation is applied
    bool hasVariation = (m_params.hueVariation != 0 || m_params.satVariation > 0 ||
                         m_params.contrastVariation > 0 || m_params.noiseAmount > 0);

    if (!hasVariation) {
        return input.clone();
    }

    cv::Mat result = input.clone();

    // === HUE VARIATION ===
    if (m_params.hueVariation != 0) {
        cv::Mat hsv;
        cv::cvtColor(result, hsv, cv::COLOR_BGR2HSV);

        std::vector<cv::Mat> hsvChannels;
        cv::split(hsv, hsvChannels);

        // Hue shift: map -100 to +100 → -90 to +90 degrees (half the hue wheel)
        float hueShift = m_params.hueVariation * 0.9f;

        hsvChannels[0].convertTo(hsvChannels[0], CV_32F);
        hsvChannels[0] += hueShift;

        // Wrap hue values (0-180 in OpenCV HSV)
        cv::Mat hueMask;
        cv::compare(hsvChannels[0], 180.0f, hueMask, cv::CMP_GE);
        cv::subtract(hsvChannels[0], cv::Scalar(180.0f), hsvChannels[0], hueMask);
        cv::compare(hsvChannels[0], 0.0f, hueMask, cv::CMP_LT);
        cv::add(hsvChannels[0], cv::Scalar(180.0f), hsvChannels[0], hueMask);
        hsvChannels[0].convertTo(hsvChannels[0], CV_8U);

        cv::merge(hsvChannels, hsv);
        cv::cvtColor(hsv, result, cv::COLOR_HSV2BGR);
    }

    // === SATURATION VARIATION ===
    if (m_params.satVariation > 0) {
        cv::Mat hsv;
        cv::cvtColor(result, hsv, cv::COLOR_BGR2HSV);

        std::vector<cv::Mat> hsvChannels;
        cv::split(hsv, hsvChannels);

        // Saturation factor: 0-100 maps to 1.0-2.0× (boost only)
        float satFactor = 1.0f + (m_params.satVariation / 100.0f);

        hsvChannels[1].convertTo(hsvChannels[1], CV_32F);
        hsvChannels[1] *= satFactor;
        cv::threshold(hsvChannels[1], hsvChannels[1], 255.0, 255.0, cv::THRESH_TRUNC);
        hsvChannels[1].convertTo(hsvChannels[1], CV_8U);

        cv::merge(hsvChannels, hsv);
        cv::cvtColor(hsv, result, cv::COLOR_HSV2BGR);
    }

    // === CONTRAST VARIATION ===
    if (m_params.contrastVariation > 0) {
        // Contrast factor: 0-100 maps to 1.0-2.0×
        float contrastFactor = 1.0f + (m_params.contrastVariation / 100.0f);

        result.convertTo(result, CV_32F);
        result = (result - 128.0f) * contrastFactor + 128.0f;
        cv::threshold(result, result, 255.0, 255.0, cv::THRESH_TRUNC);
        cv::threshold(result, result, 0.0, 0.0, cv::THRESH_TOZERO);
        result.convertTo(result, CV_8U);
    }

    // === NOISE ===
    if (m_params.noiseAmount > 0) {
        cv::Mat noise(input.size(), CV_32FC3);

        // Use a fixed seed for reproducible noise, varied by noise amount
        std::mt19937 rng(m_params.noiseAmount * 12345);
        cv::randn(noise, 0, m_params.noiseAmount * 0.5f);  // 0-50 noise std dev

        cv::Mat resultFloat;
        result.convertTo(resultFloat, CV_32FC3);
        resultFloat += noise;
        cv::threshold(resultFloat, resultFloat, 255.0, 255.0, cv::THRESH_TRUNC);
        cv::threshold(resultFloat, resultFloat, 0.0, 0.0, cv::THRESH_TOZERO);
        resultFloat.convertTo(result, CV_8UC3);
    }

    return result;
}

cv::Mat StyleTransfer::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "❌ Error in StyleTransfer: empty input image\n";
        return cv::Mat();
    }

    if (net.empty()) {
        std::cerr << "❌ Error in StyleTransfer: no model loaded\n";
        return srcImg;
    }

    // convert to 8 bit, if 16 bit image
    cv::Mat img8;
    if (srcImg.depth() == CV_16U) {
        srcImg.convertTo(img8, CV_8U, 0.00389);
    } else {
        img8 = srcImg.clone();
    }

    // convert to 3 channels if gray image
    if (img8.channels() == 1) {
        cv::cvtColor(img8, img8, cv::COLOR_GRAY2BGR);
    } else if (img8.channels() == 4) {
        cv::cvtColor(img8, img8, cv::COLOR_BGRA2BGR);
    }

    // Apply variations to input before style transfer
    cv::Mat variedInput = applyVariation(img8);

    // preprocessing for ONNX
    cv::Mat inputBlob = cv::dnn::blobFromImage(variedInput, 1.0, cv::Size(224, 224),
                                               cv::Scalar(0, 0, 0), true, true);

    // INFERENCE
    net.setInput(inputBlob, "input1");
    cv::Mat outputTensor = net.forward("output1");

    // Postprocessing
    cv::Mat result = postprocess(outputTensor, 224, 224);

    // Resize to original size
    cv::resize(result, result, srcImg.size());

    return result;
}