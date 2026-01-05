#include "style_transfer.h"

#include <opencv2/core/hal/interface.h>

#include <iostream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/opencv.hpp>
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
        "AI_models/",                // Standard relative path (dev/bin)
        "../Resources/AI_models/",   // macOS Bundle Resources
        "../../../AI_models/",       // Fallback relative to bin if run from bundle
        "../AI_models/"              // Sibling directory
    };

    std::string fullPath;
    bool found = false;

    for (const auto& dir : baseDirs) {
        std::string testPath = dir + modelName;
        // Simple check if file exists using OpenCV (or just try loading)
        // Here we rely on try/catch logic below, but strictly we should check existence first or iterate.
        // Let's assume we find it if the file handles open. But cv::dnn::readNetFromONNX throws.
        // A better way is to use <filesystem> or just access.
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
        // Fallback to default if not found (let exception handler catch it)
        fullPath = "AI_models/" + modelName;
        std::cerr << "⚠️ Could not locate model " << modelName << " in any expected path." << std::endl;
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

// convert to interleaved BGR because output of the models ist NCHW (Planar RGB)
cv::Mat StyleTransfer::postprocess(const cv::Mat& outputTensor, int rows, int cols) {
    // Output is 1x3x224x224
    // We can extract the channel pointers.
    // The data is contiguous float.
    const float* data = (float*)outputTensor.data;

    std::vector<cv::Mat> channels;
    int channelSize = rows * cols;

    for (int i = 0; i < 3; i++) {
        // pointer auf the start point of each chanel
        const float* channelData = data + (i * channelSize);

        // create a matrix
        cv::Mat channel(rows, cols, CV_32F, const_cast<float*>(channelData));

        channels.push_back(channel);
    }

    // merge these 3 channels to a float image
    cv::Mat mergedImg;
    cv::merge(channels, mergedImg);

    // clamp the value 0 - 255
    cv::threshold(mergedImg, mergedImg, 255.0, 255.0, cv::THRESH_TRUNC);
    cv::threshold(mergedImg, mergedImg, 0.0, 0.0, cv::THRESH_TOZERO);

    // convert to 8bit
    cv::Mat img8;
    mergedImg.convertTo(img8, CV_8U);

    // RGB to BGR
    cv::cvtColor(img8, img8, cv::COLOR_RGB2BGR);

    return img8;
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
    }

    else if (img8.channels() == 4) {
        cv::cvtColor(img8, img8, cv::COLOR_BGRA2BGR);
    }

    // preprocessing for ONNX
    cv::Mat inputBlob =
        cv::dnn::blobFromImage(img8,
                               1.0,                 // value scale factor 255 x 1 = 255
                               cv::Size(224, 224),  // transfer models expect 224x224
                               cv::Scalar(0, 0, 0),
                               true,   // transfer models expect RGB images
                               true);  // crop

    //================ INFERENZ ===============

    net.setInput(inputBlob, "input1");
    cv::Mat outputTensor = net.forward("output1");

    // ========== Posprocessing (Tensor -> Bild)===========

    cv::Mat result = postprocess(outputTensor, 224, 224);

    // Resize to original size
    cv::resize(result, result, srcImg.size());

    return result;
}