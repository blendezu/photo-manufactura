#include "image_utils.h"

#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

#include "color_space.h"

std::tuple<float, float> ImageUtils::calculateMinMax(const cv::Mat& srcImg, int channel) {
    if (srcImg.empty()) {
        std::cerr << "Error in getMinMaxHSL: empty input image\n";
        return {0.0f, 0.0f};
    }

    if (channel < 0 || channel >= srcImg.channels()) {
        std::cerr << "Error in getMinMaxHSL: invalid channel index\n";
        return {0.0f, 0.0f};
    }

    double minVal = 0.0, maxVal = 0.0;

    if (srcImg.channels() == 1) {
        float maxRange = 0.0f;
        if (srcImg.depth() == CV_8U) {
            maxRange = 255.0f;
        } else {
            maxRange = 65535.0f;
        }
        cv::minMaxLoc(srcImg, &minVal, &maxVal);
        return {minVal / maxRange, maxVal / maxRange};
    }

    cv::Mat ch;
    cv::extractChannel(srcImg, ch, channel);

    cv::minMaxLoc(ch, &minVal, &maxVal);
    return {static_cast<float>(minVal), static_cast<float>(maxVal)};
}

ImageUtils::WeightParams ImageUtils::precalculateWhiteWeightParams(float minVal, float maxVal,
                                                                   float underP, float upperP) {
    ImageUtils::WeightParams params;
    params.underVal = minVal + (maxVal - minVal) * underP;
    params.upperVal = minVal + (maxVal - minVal) * upperP;

    const float range = params.upperVal - params.underVal;
    params.constantWeight = (range <= 1e-6f);  // to avoid 0 division
    params.invRange = params.constantWeight ? 0.0f : (1.0f / range);
    return params;
}

float ImageUtils::calculateBrightWeight(float currVal, WeightParams params) {
    if (currVal <= params.underVal) {
        return 0.0f;
    } else if (currVal >= params.upperVal) {
        return 1.0f;
    }

    const float t = (params.upperVal - currVal) * params.invRange;
    return fma(-t, t, 1.0f);
}

ImageUtils::WeightParams ImageUtils::precalculateDarkWeightParams(float minVal, float maxVal,
                                                                  float underP, float upperP) {
    ImageUtils::WeightParams params;
    params.underVal = minVal + (maxVal - minVal) * underP;
    params.upperVal = minVal + (maxVal - minVal) * upperP;

    const float range = params.upperVal - params.underVal;
    params.constantWeight = (range <= 1e-6f);  // to avoid 0 division
    params.invRange = params.constantWeight ? 0.0f : (1.0f / range);

    return params;
}

float ImageUtils::calculateDarkWeight(float currVal, const WeightParams& params) {
    if (currVal <= params.underVal) {
        return 1.0f;
    }

    if (currVal >= params.upperVal) {
        return 0.0f;
    }

    const float x = (currVal - params.underVal) * params.invRange;
    return fma(-x, x, 1.0f);
}

cv::Mat ImageUtils::blendScratch(const cv::Mat& srcImg, cv::Mat& scratchImg) {
    if (scratchImg.empty() || srcImg.empty()) {
        std::cerr << "Error in blendScratch: empty input images\n";
        return cv::Mat();
    }

    if (srcImg.type() == CV_8UC3) {
        return blendScratchRGBTemplate<cv::Vec3b>(srcImg, scratchImg);
    }

    else if (srcImg.type() == CV_16UC3) {
        return blendScratchRGBTemplate<cv::Vec3w>(srcImg, scratchImg);
    }

    else if (srcImg.type() == CV_8UC1) {
        return blendScratchGrayTemplate<uchar>(srcImg, scratchImg);
    }

    else if (srcImg.type() == CV_16UC1) {
        return blendScratchGrayTemplate<ushort>(srcImg, scratchImg);
    } else {
        std::cerr << "Error in blendScratch: unsupported image type\n";
        return cv::Mat();
    }
}

cv::Mat ImageUtils::setSaturationTo(const cv::Mat& srcImg, float sat) {
    auto start = std::chrono::high_resolution_clock::now();
    if (srcImg.empty()) {
        std::cerr << "❌Error in setSaturationTo: empty input image\n";
        return cv::Mat();
    }

    if (sat < 0 || sat > 1) {
        std::cerr << "❌Error in setSaturationTo: 0 <= sat <= 1\n";
        return cv::Mat();
    }

    if (srcImg.type() != CV_8UC3 && srcImg.type() != CV_16UC3) {
        return srcImg;
    }

    cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

    // clang-format off
    #pragma omp parallel for
    // clang-format on

    for (int y = 0; y < hslImg.rows; y++) {
        cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);

        for (int x = 0; x < hslImg.cols; x++) {
            float H = hslPtr[x][0];
            float L = hslPtr[x][2];

            hslPtr[x] = cv::Vec3f(H, sat, L);
        }
    }
    if (srcImg.depth() == CV_8U) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "set saturation Time: " << duration.count() << " ms" << std::endl;
        return ColorSpace::convertHSL2BGR(hslImg, 8);
    } else {
        return ColorSpace::convertHSL2BGR(hslImg, 16);
    }
}

cv::Mat ImageUtils::setVintageWarm(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "❌Error int setVintageWarm: empty input image\n";
        return cv::Mat();
    }

    if (srcImg.type() != CV_8UC3 && srcImg.type() != CV_16UC3) {
        std::cerr << "❌Error int setVintageWarm: unsupported image type\n";
        return cv::Mat();
    }

    if (srcImg.type() == CV_8UC3) {
        return setVintageWarmBGR<cv::Vec3b>(srcImg);
    }

    else if (srcImg.type() == CV_16UC3) {
        return setVintageWarmBGR<cv::Vec3w>(srcImg);
    }

    else {
        std::cerr << "Error in SetVintageWarm: unsupported image type\n";
        return cv::Mat();
    }
}

cv::Mat ImageUtils::createThumbnail(const cv::Mat& srcImg, int targetSize) {
    if (srcImg.empty()) {
        std::cout << "[createThumbnail] ❌ Error: The input image is empty\n";
        return cv::Mat();
    }

    // Check if the image is alrealy smaller than targetSize
    if (srcImg.rows <= targetSize && srcImg.cols <= targetSize) {
        return srcImg.clone();
    }

    cv::Mat dstImg;
    cv::resize(srcImg, dstImg, cv::Size(targetSize, targetSize), 0, 0, cv::INTER_AREA);

    return dstImg;
}