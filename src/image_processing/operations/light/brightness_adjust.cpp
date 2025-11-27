#include "brightness_adjust.h"

#include <opencv2/core/hal/interface.h>

#include <opencv2/opencv.hpp>

#include "color_space.h"

cv::Mat AdjustBrightness::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in BrightnessAdjust: the input image is empty\n";
        return cv::Mat();
    }

    // =========================================================
    // 1. BGR Image (via HSL)
    // =========================================================
    if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
        // 1. Convert BGR to HSL
        cv::Mat hslImg = ColorSpace::convertBGR2HSL(srcImg);

        const float deltaL = brightness / 100.0f;

        // Change factor
        const float factor = 1.0f + deltaL;

        // 2. Parallelizing across lines
        // Modify direct the hslImg instead a temporary image to save Memory
        // clang-format off
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < hslImg.rows; y++) {
            float* __restrict ptr = hslImg.ptr<float>(y);

            int len = hslImg.cols * 3;

            // no #pragma omp simd --> because of inefficiency
            for (int i = 2; i < len; i += 3) {  // Only L-Chanel (Index 2, 5, 8...)

                float L = ptr[i];
                float newVal = L * factor;

                newVal = std::clamp(newVal, 0.0f, 1.0f);

                ptr[i] = newVal;
            }
        }

        // 3. convert to BGR image
        int depth = (srcImg.type() == CV_8UC3) ? 8 : 16;
        return ColorSpace::convertHSL2BGR(hslImg, depth);
    }

    // =========================================================
    // 2. Grayscale 8-bit (CV_8UC1)
    // =========================================================
    else if (srcImg.type() == CV_8UC1) {
        return grayImgTemplate<uchar>(srcImg, brightness);
    }

    // =========================================================
    // 3. Grayscale 16-bit (CV_16UC1)
    // =========================================================
    else if (srcImg.type() == CV_16UC1) {
        return grayImgTemplate<ushort>(srcImg, brightness);
    }

    else {
        std::cerr << "Error: unsupported image type\n";
        return cv::Mat();
    }
}

// #include "brightness_adjust.h"

// #include <algorithm>
// #include <opencv2/core/mat.hpp>
// #include <opencv2/core/saturate.hpp>
// #include <opencv2/opencv.hpp>

// #include "color_space.h"

// cv::Mat BrightnessAdjust::apply(const cv::Mat& srcImg) {
//     if (srcImg.empty()) {
//         std::cerr << "Error in BrightnessAdjust: the input image is empty\n";
//         return cv::Mat();
//     }

//     // RGB image
//     if (srcImg.type() == CV_8UC3 || srcImg.type() == CV_16UC3) {
//         ColorSpace ColorSpace;
//         cv::Mat hslImg = ColorSpace.convertBGR2HSL(srcImg);
//         auto start = std::chrono::high_resolution_clock::now();

//         cv::Mat dstImg(hslImg.size(), hslImg.type());  // output image

//         float deltaL = brightness / 100.0f;

//         for (int y = 0; y < hslImg.rows; y++) {
//             const cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);
//             cv::Vec3f* dstPtr = dstImg.ptr<cv::Vec3f>(y);

//             for (int x = 0; x < hslImg.cols; x++) {
//                 float H = hslPtr[x][0];
//                 float S = hslPtr[x][1];
//                 float currL = hslPtr[x][2];
//                 float newL = std::clamp(currL * (1.0f + deltaL), 0.0f, 1.0f);

//                 dstPtr[x] = cv::Vec3f(H, S, newL);
//             }
//         }
//         if (srcImg.type() == CV_8UC3) {
//             auto end = std::chrono::high_resolution_clock::now();
//             auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
//             std::cout << duration.count() << std::endl;
//             return ColorSpace.convertHSL2BGR(dstImg, 8);
//         } else {
//             return ColorSpace.convertHSL2BGR(dstImg, 16);
//         }
//     }

//     // gray image
//     else if (srcImg.type() == CV_8UC1) {
//         cv::Mat dstImg(srcImg.size(), srcImg.type());
//         for (int y = 0; y < srcImg.rows; y++) {
//             const uchar* srcPtr = srcImg.ptr<uchar>(y);
//             uchar* dstPtr = dstImg.ptr<uchar>(y);

//             for (int x = 0; x < srcImg.cols; x++) {
//                 dstPtr[x] = std::clamp(srcPtr[x] + brightness, 0, 255);
//             }
//         }
//         return dstImg;
//     }

//     else if (srcImg.type() == CV_16UC1) {
//         cv::Mat dstImg(srcImg.size(), srcImg.type());
//         for (int y = 0; y < srcImg.rows; y++) {
//             const ushort* srcPtr = srcImg.ptr<ushort>(y);
//             ushort* dstPtr = dstImg.ptr<ushort>(y);

//             for (int x = 0; x < srcImg.cols; x++) {
//                 int adjusted = srcPtr[x] + static_cast<int>(brightness * 257);
//                 dstPtr[x] = cv::saturate_cast<ushort>(adjusted);
//             }
//         }

//         return dstImg;
//     }

//     else {
//         std::cerr << "Error: unsupported image type\n";
//         return cv::Mat();
//     }
// }
