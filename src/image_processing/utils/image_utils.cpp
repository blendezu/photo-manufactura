#include "image_utils.h"

#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

#include "color_space.h"

std::tuple<float, float> ImageUtils::caculateMinMax(const cv::Mat& srcImg, int channel) {
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

float ImageUtils::caculateBrightWeight(float currVal, float minVal, float maxVal, double underP,
                                       double upperP) {
    float underVal = minVal + (maxVal - minVal) * underP;
    float upperVal = minVal + (maxVal - minVal) * upperP;
    float weight = 0.0f;

    if (currVal <= underVal) {
        weight = 0.0f;
    } else if (currVal <= upperVal) {
        float t = (upperVal - currVal) / (upperVal - underVal);
        weight = 1 - t * t;
    } else {
        weight = 1.0f;
    }
    return weight;
}

float ImageUtils::caculateDarkWeight(float currVal, float minVal, float maxVal, double underP,
                                     double upperP) {
    float underVal = minVal + (maxVal - minVal) * underP;
    float upperVal = minVal + (maxVal - minVal) * upperP;
    float weight = 0.0f;

    if (currVal <= underVal) {
        weight = 1.0f;
    } else if (currVal <= upperVal) {
        float x = (currVal - underP) / (upperVal - underVal);
        weight = 1 - x * x;
    } else {
        weight = 0.0f;
    }
    return weight;
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
    cv::Mat dstImgHSL(hslImg.size(), hslImg.type());

    for (int y = 0; y < hslImg.rows; y++) {
        const cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);
        cv::Vec3f* dstPtr = dstImgHSL.ptr<cv::Vec3f>(y);

        for (int x = 0; x < hslImg.cols; x++) {
            float H = hslPtr[x][0];
            float L = hslPtr[x][2];

            dstPtr[x] = cv::Vec3f(H, sat, L);
        }
    }
    if (srcImg.depth() == CV_8U) {
        return ColorSpace::convertHSL2BGR(dstImgHSL, 8);
    } else {
        return ColorSpace::convertHSL2BGR(dstImgHSL, 16);
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

double ImageUtils::calculateCubicWeight(double t) {
    t = std::abs(t);

    if (t < 1) {
        return 1.5 * t * t * t - 2.5 * t * t + 1;
    } else if (1 <= t && t < 2) {
        return -0.5 * t * t * t + 2.5 * t * t - 4 * t + 2;
    } else {
        return 0;
    }
}