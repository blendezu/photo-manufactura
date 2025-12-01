#include "style_transfer.h"

#include <opencv2/core/hal/interface.h>

#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/dnn/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

StyleTransfer::StyleTransfer(StyleType style)
    : currentStyle(style), env(ORT_LOGGING_LEVEL_WARNING, "StyleTransfer") {
    // Standard-Optionen
    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);

    // Erstes Modell laden
    loadModel(currentStyle);
}

void StyleTransfer::loadModel(StyleType style) {
    currentStyle = style;

    std::string baseDir = "AI_models/";
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

    std::string fullPath = baseDir + modelName;

    std::cout << "Loading model from " << fullPath << std::endl;

    try {
        session = std::make_unique<Ort::Session>(env, fullPath.c_str(), sessionOptions);
    } catch (const Ort::Exception& e) {
        std::cerr << "❌ ONNX Error loading model: " << e.what() << std::endl;
        std::cerr << "Tried path: " << fullPath << std::endl;
        session = nullptr;
    }
}

void StyleTransfer::setStyle(StyleType style) {
    if (currentStyle != style) {
        loadModel(style);
    }
}

// convert to interleaved BGR because output of the models ist NCHW (Planar RGB)
cv::Mat StyleTransfer::postprocess(const std::vector<float>& floatArr, int rows, int cols) {
    std::vector<cv::Mat> channels;
    int channelSize = rows * cols;

    for (int i = 0; i < 3; i++) {
        // pointer auf the start point of each chanel
        const float* data = floatArr.data() + (i * channelSize);

        // create a matrix
        cv::Mat channel(rows, cols, CV_32F, const_cast<float*>(data));

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

    if (!session) {
        std::cerr << "❌ Error in StyleTransfer: no ONNX session loaded\n";
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
                               true,  // transfer models expect RGB images
                               true);

    //================ INFERENZ ===============
    // tensor setup
    size_t inputTensorSize = inputBlob.total();
    std::vector<float> inputTensorValues(inputTensorSize);

    // copy data from Blob to the vector
    std::memcpy(inputTensorValues.data(), inputBlob.ptr<float>(), inputTensorSize * sizeof(float));

    std::vector<int64_t> inputShape = {1, 3, 224, 224};

    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor =
        Ort::Value::CreateTensor<float>(memoryInfo, inputTensorValues.data(), inputTensorSize,
                                        inputShape.data(), inputShape.size());

    // name for in & output
    const char* inputNames[] = {"input1"};
    const char* outputNames[] = {"output1"};

    // Run
    auto outputTensors =
        session->Run(Ort::RunOptions{nullptr}, inputNames, &inputTensor, 1, outputNames, 1);

    // ========== Posprocessing (Tensor -> Bild)===========
    float* floatArr = outputTensors.front().GetTensorMutableData<float>();

    cv::Mat result =
        postprocess(std::vector<float>(floatArr, floatArr + inputTensorSize), 224, 224);

    // Resize to original size
    cv::resize(result, result, srcImg.size());

    return result;
}