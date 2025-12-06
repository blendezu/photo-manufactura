#include "AI_denoise.h"  // Achte darauf: #include "denoise/denoise.h" falls nötig

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>

// Konstruktor
AiDenoise::AiDenoise(int strengthValue)
    : env(ORT_LOGGING_LEVEL_WARNING, "Denoise"), strength(std::clamp(strengthValue, 0, 100)) {
    // Performance Optionen für ONNX Runtime
    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);

    loadModel();
}

void AiDenoise::setStrength(int value) {
    strength = std::clamp(value, 0, 100);
}

void AiDenoise::loadModel() {
    // Pfad anpassen, falls nötig
    std::string modelPath = "AI_models/1xDeNoise_realplksr_otf_fp32.onnx";

    if (!std::filesystem::exists(modelPath)) {
        std::cerr << "❌ Error: Denoise Model not found at: " << modelPath << std::endl;
        return;
    }

    std::cout << "✅ Loading denoise model (ONNX Runtime) ...\n";

    try {
#ifdef _WIN32
        std::wstring widePath = std::wstring(modelPath.begin(), modelPath.end());
        session = std::make_unique<Ort::Session>(env, widePath.c_str(), sessionOptions);
#else
        session = std::make_unique<Ort::Session>(env, modelPath.c_str(), sessionOptions);
#endif
    } catch (const Ort::Exception& e) {
        std::cerr << "❌ ONNX Runtime Error: " << e.what() << std::endl;
        session = nullptr;
    }
}

// ---------------------------------------------------------
// 1. PREPROCESS: Bild -> OpenCV Mat (8-Bit BGR)
// ---------------------------------------------------------
cv::Mat AiDenoise::preprocess(const cv::Mat& srcImg) {
    cv::Mat img8;
    // 16-Bit zu 8-Bit
    if (srcImg.depth() == CV_16U) {
        srcImg.convertTo(img8, CV_8U, 255.0 / 65535.0);
    } else {
        img8 = srcImg.clone();
    }

    // Kanäle korrigieren
    if (img8.channels() == 4)
        cv::cvtColor(img8, img8, cv::COLOR_BGRA2BGR);
    else if (img8.channels() == 1)
        cv::cvtColor(img8, img8, cv::COLOR_GRAY2BGR);

    return img8;
}

// ---------------------------------------------------------
// 2. APPLY: Der Ablauf
// ---------------------------------------------------------
cv::Mat AiDenoise::apply(const cv::Mat& srcImg) {
    auto start = std::chrono::high_resolution_clock::now();
    if (srcImg.empty() || !session)
        return srcImg;
    if (strength == 0)
        return srcImg;

    // A. Vorbereitung
    cv::Mat preprocessedImg = preprocess(srcImg);

    // B. Blob erstellen (OpenCV hilft uns hier beim Layout NCHW)
    // Wir nutzen blobFromImage nur zur Datenformatierung, nicht zur Inferenz
    cv::Mat blob = cv::dnn::blobFromImage(preprocessedImg,
                                          1.0 / 255.0,  // Scale 0..1 Float
                                          cv::Size(),   // Originalgröße
                                          cv::Scalar(0, 0, 0),
                                          true,  // SwapRB (BGR -> RGB)
                                          false);

    // C. ONNX Runtime Input Tensor erstellen
    size_t inputTensorSize = blob.total();
    std::vector<float> inputValues(blob.ptr<float>(), blob.ptr<float>() + inputTensorSize);

    // Dimensionen: [1, 3, H, W]
    std::vector<int64_t> inputShape = {1, 3, blob.size[2], blob.size[3]};

    auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo, inputValues.data(), inputTensorSize, inputShape.data(), inputShape.size());

    // D. Run Inference
    const char* inputNames[] = {"input"};
    const char* outputNames[] = {"output"};

    try {
        auto outputTensors =
            session->Run(Ort::RunOptions{nullptr}, inputNames, &inputTensor, 1, outputNames, 1);

        // E. Postprocessing
        float* floatArr = outputTensors.front().GetTensorMutableData<float>();

        // Da es 1xDeNoise ist, entspricht Output-Größe der Input-Größe
        int rows = static_cast<int>(inputShape[2]);
        int cols = static_cast<int>(inputShape[3]);

        cv::Mat denoisedImg = postprocess(std::vector<float>(floatArr, floatArr + inputTensorSize),
                                          rows, cols, preprocessedImg.size());

        // F. Blending
        cv::Mat dstImg;
        float alpha = static_cast<float>(strength) / 100.0f;
        cv::addWeighted(preprocessedImg, 1.0f - alpha, denoisedImg, alpha, 0.0, dstImg);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << duration.count() << std::endl;
        return dstImg;

    } catch (const Ort::Exception& e) {
        std::cerr << "❌ Inference Error: " << e.what() << std::endl;
        return srcImg;
    }
}

// ---------------------------------------------------------
// 3. POSTPROCESS: Float Vektor -> Bild
// ---------------------------------------------------------
cv::Mat AiDenoise::postprocess(const std::vector<float>& outputTensorData, int rows, int cols,
                               const cv::Size& targetSize) {
    // NCHW -> HWC
    std::vector<cv::Mat> planes;
    int planeSize = rows * cols;

    for (int i = 0; i < 3; i++) {
        // const_cast ist ok, da wir nur lesen und in eine neue Mat kopieren
        const float* dataPtr = outputTensorData.data() + (i * planeSize);
        cv::Mat plane(rows, cols, CV_32F, const_cast<float*>(dataPtr));
        planes.push_back(plane);
    }

    cv::Mat resultFloat;
    cv::merge(planes, resultFloat);

    // Float 0..1 -> 8-Bit 0..255
    cv::Mat result8u;
    resultFloat.convertTo(result8u, CV_8U, 255.0);

    // RGB -> BGR
    cv::cvtColor(result8u, result8u, cv::COLOR_RGB2BGR);

    // Safety Resize
    if (result8u.size() != targetSize) {
        cv::resize(result8u, result8u, targetSize);
    }

    return result8u;
}