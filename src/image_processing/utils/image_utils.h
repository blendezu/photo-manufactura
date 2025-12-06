#pragma once
#include <opencv2/core/hal/interface.h>

#include <cstdlib>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <tuple>

class ImageUtils {
   private:
    struct WeightParams {
        float underVal;
        float upperVal;
        float invRange;
        bool constantWeight;
    };

   public:
    static std::tuple<float, float> calculateMinMax(const cv::Mat& scrImg, int channel);

    static WeightParams precalculateWhiteWeightParams(float minVal, float maxVal, float underP,
                                                      float upperP);

    static float calculateBrightWeight(float currVal, WeightParams params);

    static WeightParams precalculateDarkWeightParams(float minVal, float maxVal, float underP,
                                                     float upperP);

    static float calculateDarkWeight(float currVal, const WeightParams& params);

    static cv::Mat blendScratch(const cv::Mat& srcImg, cv::Mat& scratchImg);

    static cv::Mat setSaturationTo(const cv::Mat& scrImg, float sat);

    static cv::Mat setVintageWarm(const cv::Mat& srcImg);

    static inline float calculateCubicWeight(double t) {
        t = std::abs(t);

        if (t < 1) {
            return 1.5 * t * t * t - 2.5 * t * t + 1;
        } else if (1 <= t && t < 2) {
            return -0.5 * t * t * t + 2.5 * t * t - 4 * t + 2;
        } else {
            return 0;
        }
    }

    static cv::Mat gaussianBlur(const cv::Mat& srcImg, int kernelSize, double sigma);

    static cv::Mat copyMakeBorder(const cv::Mat& srcImg, int borderSize);

    static cv::Mat createThumbnail(const cv::Mat& srcImg, int targetSize = 512);

   private:
    template <typename T>
    static cv::Mat blendScratchRGBTemplate(const cv::Mat srcImg, cv::Mat scratchImg) {
        if (srcImg.empty() || scratchImg.empty()) {
            std::cerr << "Error in blendScratchRGBTemplate: Empty input images\n";
            return cv::Mat();
        }

        if (srcImg.type() == CV_8UC1 || srcImg.type() == CV_16UC1) {
            std::cerr << "Error in blendScratchRGBTemplate: Empty input images\n";
            return cv::Mat();
        }

        // resize the scratchImg to srcImg
        cv::resize(scratchImg, scratchImg, cv::Size(srcImg.cols, srcImg.rows));

        cv::Mat dstImg = srcImg.clone();  // output image

#pragma omp parallel for
        for (int y = 0; y < srcImg.rows; y++) {
            // const T* srcPtr = srcImg.ptr<T>(y);
            const cv::Vec3b* scratchPtr = scratchImg.ptr<cv::Vec3b>(y);
            T* dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                int vB = scratchPtr[x][0];
                int vG = scratchPtr[x][1];
                int vR = scratchPtr[x][2];

                if (vB > 200 && vG > 200 && vR > 200) {
                    if (srcImg.depth() == CV_8U) {
                        uchar white = 250;
                        dstPtr[x] = T(white, white, white);
                    } else {
                        ushort white = 65000;
                        dstPtr[x] = T(white, white, white);
                    }
                }
            }
        }
        return dstImg;
    }

    template <typename T>
    static cv::Mat blendScratchGrayTemplate(const cv::Mat srcImg, cv::Mat scratchImg) {
        if (srcImg.empty() || scratchImg.empty()) {
            std::cerr << "❌Error in blendScratchGrayTemplate: empty input image\n";
            return cv::Mat();
        }

        if (srcImg.type() != CV_8UC1 && srcImg.type() != CV_16UC1) {
            std::cerr << "❌Error in blendScratchGrayTemplate: unsupported image type\n";
            return cv::Mat();
        }

        // convert scratch image to gray image, if still not
        if (scratchImg.channels() != 1) {
            cv::cvtColor(scratchImg, scratchImg, cv::COLOR_BGR2GRAY);
        }

        // resize the scratch image to size of src image
        cv::resize(scratchImg, scratchImg, cv::Size(srcImg.cols, srcImg.rows));

        cv::Mat dstImg = srcImg.clone();  // output image

        int white = 0;
        (srcImg.depth() == CV_16U) ? white = 65000 : white = 205;

#pragma omp parallel for
        for (int y = 0; y < srcImg.rows; y++) {
            const uchar* scratchPtr = scratchImg.ptr<uchar>(y);
            T* dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                if (scratchPtr[x] > 200) {
                    if (srcImg.depth() == CV_16U) {
                        dstPtr[x] = white;
                    } else {
                        dstPtr[x] = white;
                    }
                }
            }
        }

        return dstImg;
    }

    template <typename T>
    static cv::Mat setVintageWarmBGR(const cv::Mat& srcImg) {
        cv::Mat dstImg(srcImg.size(), srcImg.type());

        int maxRange = 0;
        (srcImg.depth() == CV_16U) ? maxRange = 65535 : maxRange = 255;

#pragma omp parallel for
        for (int y = 0; y < srcImg.rows; y++) {
            const T* srcPtr = srcImg.ptr<T>(y);
            T* dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                int B = srcPtr[x][0] * 992 >> 10;
                int G = srcPtr[x][1] * 1075 >> 10;
                int R = srcPtr[x][2] * 1126 >> 10;

                B = std::clamp(B, 0, maxRange);
                G = std::clamp(G, 0, maxRange);
                R = std::clamp(R, 0, maxRange);

                dstPtr[x] = T(B, G, R);
            }
        }
        return dstImg;
    }
};