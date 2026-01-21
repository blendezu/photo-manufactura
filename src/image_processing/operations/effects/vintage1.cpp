#include "vintage1.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

// Initialize static members
cv::Mat Vintage1::scratchImg = cv::imread("images/9003.jpg");
cv::Mat Vintage1::cachedScratchMask;
cv::Size Vintage1::lastSize;

cv::Mat Vintage1::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in Vintage1: empty input image\n";
        return cv::Mat();
    }

    // Lazy load scratch image if it failed previously or wasn't loaded
    if (scratchImg.empty()) {
        std::vector<std::string> potentialPaths = {
            "images/9003.jpg",                 // Standard relative path (dev mode)
            "../Resources/images/9003.jpg",    // macOS Bundle Resources (relative to MacOS dir)
            "../../../images/9003.jpg",        // Fallback relative to bin if run from bundle
            "../images/9003.jpg"               // Another common relative path
        };

        for (const auto& path : potentialPaths) {
            scratchImg = cv::imread(path);
            if (!scratchImg.empty()) {
                std::cout << "[Vintage1] Loaded scratch texture from: " << path << std::endl;
                break;
            }
        }
        
        if (scratchImg.empty()) {
             std::cerr << "[Vintage1] ❌ Failed to load scratch texture '9003.jpg' from any known location!" << std::endl;
        }
    }

    // Update Cache if size changed
    if (!scratchImg.empty() && (srcImg.size() != lastSize || cachedScratchMask.empty())) {
        cv::Mat resizedScratch;
        cv::resize(scratchImg, resizedScratch, srcImg.size());

        cachedScratchMask.create(srcImg.size(), CV_8UC1);

        const int threshold = 200;

#pragma omp parallel for
        for (int y = 0; y < resizedScratch.rows; y++) {
            const cv::Vec3b* srcPtr = resizedScratch.ptr<cv::Vec3b>(y);
            uchar* maskPtr = cachedScratchMask.ptr<uchar>(y);
            for (int x = 0; x < resizedScratch.cols; x++) {
                if (srcPtr[x][0] > threshold && srcPtr[x][1] > threshold &&
                    srcPtr[x][2] > threshold) {
                    maskPtr[x] = 255;
                } else {
                    maskPtr[x] = 0;
                }
            }
        }
        lastSize = srcImg.size();
    }

    cv::Mat dstImg;
    dstImg.create(srcImg.size(), srcImg.type());

    if (srcImg.channels() == 3) {
        if (srcImg.depth() == CV_8U) {
#pragma omp parallel for
            for (int y = 0; y < srcImg.rows; y++) {
                const cv::Vec3b* sPtr = srcImg.ptr<cv::Vec3b>(y);
                cv::Vec3b* dPtr = dstImg.ptr<cv::Vec3b>(y);
                const uchar* mPtr =
                    cachedScratchMask.empty() ? nullptr : cachedScratchMask.ptr<uchar>(y);

                for (int x = 0; x < srcImg.cols; x++) {
                    if (mPtr && mPtr[x]) {
                        dPtr[x] = cv::Vec3b(250, 250, 250);  // White scratch
                    } else {
                        int b = sPtr[x][0];
                        int g = sPtr[x][1];
                        int r = sPtr[x][2];
                        
                        // Use HSL Lightness = (Max + Min) / 2 to match original desaturation
                        int minVal = std::min({b, g, r});
                        int maxVal = std::max({b, g, r});
                        int gray = (minVal + maxVal) / 2;

                        // Warm Tint
                        int outB = (gray * 992) >> 10;
                        int outG = (gray * 1075) >> 10;
                        int outR = (gray * 1126) >> 10;

                        dPtr[x][0] = std::min(255, outB);
                        dPtr[x][1] = std::min(255, outG);
                        dPtr[x][2] = std::min(255, outR);
                    }
                }
            }
        } else if (srcImg.depth() == CV_16U) {
#pragma omp parallel for
            for (int y = 0; y < srcImg.rows; y++) {
                const cv::Vec3w* sPtr = srcImg.ptr<cv::Vec3w>(y);
                cv::Vec3w* dPtr = dstImg.ptr<cv::Vec3w>(y);
                const uchar* mPtr =
                    cachedScratchMask.empty() ? nullptr : cachedScratchMask.ptr<uchar>(y);

                for (int x = 0; x < srcImg.cols; x++) {
                    if (mPtr && mPtr[x]) {
                        dPtr[x] = cv::Vec3w(64000, 64000, 64000);
                    } else {
                        int b = sPtr[x][0];
                        int g = sPtr[x][1];
                        int r = sPtr[x][2];

                        int minVal = std::min({b, g, r});
                        int maxVal = std::max({b, g, r});
                        int gray = (minVal + maxVal) / 2;

                        int outB = (gray * 992) >> 10;
                        int outG = (gray * 1075) >> 10;
                        int outR = (gray * 1126) >> 10;

                        dPtr[x][0] = std::min(65535, outB);
                        dPtr[x][1] = std::min(65535, outG);
                        dPtr[x][2] = std::min(65535, outR);
                    }
                }
            }
        }
    } else {
        // Single channel case: only blend scratch
        if (srcImg.depth() == CV_8U) {
#pragma omp parallel for
            for (int y = 0; y < srcImg.rows; y++) {
                const uchar* sPtr = srcImg.ptr<uchar>(y);
                uchar* dPtr = dstImg.ptr<uchar>(y);
                const uchar* mPtr =
                    cachedScratchMask.empty() ? nullptr : cachedScratchMask.ptr<uchar>(y);

                for (int x = 0; x < srcImg.cols; x++) {
                    if (mPtr && mPtr[x]) {
                        dPtr[x] = 205;
                    } else {
                        dPtr[x] = sPtr[x];
                    }
                }
            }
        } else if (srcImg.depth() == CV_16U) {
#pragma omp parallel for
            for (int y = 0; y < srcImg.rows; y++) {
                const ushort* sPtr = srcImg.ptr<ushort>(y);
                ushort* dPtr = dstImg.ptr<ushort>(y);
                const uchar* mPtr =
                    cachedScratchMask.empty() ? nullptr : cachedScratchMask.ptr<uchar>(y);

                for (int x = 0; x < srcImg.cols; x++) {
                    if (mPtr && mPtr[x]) {
                        dPtr[x] = 65000;
                    } else {
                        dPtr[x] = sPtr[x];
                    }
                }
            }
        } else {
            // Unhandled type, return copy
             srcImg.copyTo(dstImg);
        }
    }

    return dstImg;
}