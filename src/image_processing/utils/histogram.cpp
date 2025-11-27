#include "histogram.h"

#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

//===================== 2. Implementation better Performance ======================

std::tuple<cv::Mat, cv::Mat> Histogram::histogramImg(const cv::Mat& src) {
    if (src.empty()) {
        std::cerr << "Error in Histogram: loading image failed\n";
        return {cv::Mat(), cv::Mat()};
    }

    // convert 16bit to 8
    cv::Mat img8;
    if (src.type() == CV_16UC1) {
        // 255.0 / 65535.0 = 0.00389105 to avoid division
        src.convertTo(img8, CV_8UC1, 0.00389105);
    } else if (src.type() == CV_16UC3) {
        src.convertTo(img8, CV_8UC3, 0.00389105);
    } else {
        img8 = src;
    }

    // hist dimensions
    const int histSize = 256;
    const int binW = 3;                 // Width of each bin
    const int histW = histSize * binW;  // Width of the hist
    const int histH = 2 * histW / 3;    // Height of the hist

    // luminance hist images -> gray image & only luminance for RGB image
    cv::Mat histImg(histH, histW, CV_8UC4, cv::Scalar(0, 0, 0, 0));

    // hist counter. 0:B, 1:G, 2:R, 3:L or 0:gray
    int histGlobal[4][256] = {{0}};

// ------------- Step 1: Caculate hist vectors -----------
// clang-format off
    #pragma omp parallel
    // clang-format on
    {
        int locHist[4][256] = {{0}};  // local hist to avoid Race Conditions

        // clang-format off
        #pragma omp for
        // clang-format on
        for (int y = 0; y < img8.rows; y++) {
            const uchar* ptr = img8.ptr<uchar>(y);

            // gray image
            if (img8.channels() == 1) {
                for (int x = 0; x < img8.cols; x++) {
                    locHist[0][ptr[x]]++;
                }
            } else {                      // BGR image
                int len = img8.cols * 3;  // 1D Array
                for (int x = 0; x < len; x += 3) {
                    int B = ptr[x];
                    int G = ptr[x + 1];
                    int R = ptr[x + 2];

                    // caculate luminance with bit shift for better performance
                    int lum = (R * 77 + G * 150 + B * 29) >> 8;

                    locHist[0][B]++;
                    locHist[1][G]++;
                    locHist[2][R]++;
                    locHist[3][lum]++;
                }
            }
        }

// merge --> use critical to block another threads
// clang-format off
        #pragma omp critical
        // clang-format on
        {
            for (int c = 0; c < 4; c++) {
                for (int i = 0; i < 256; i++) {
                    histGlobal[c][i] += locHist[c][i];
                }
            }
        }
    }

    // ------------------- Step 2: Normalization & Rendering

    if (img8.channels() == 1) {
        int maxVal = 0;
        for (int i = 0; i < 256; i++) {
            // search for the greatest value
            if (histGlobal[0][i] > maxVal) {
                maxVal = histGlobal[0][i];
            }

            if (maxVal == 0) {
                maxVal = 1;
            }
        }

        // create the bins by changing the value of each pixel
        float invBinW = 1.0f / binW;  // to avoid division in the loop
        float binScale = 1.0f / maxVal;

        // clang-format off
        #pragma omp parallel for
        // clang-format on

        for (int x = 0; x < histW; x++) {
            int binIdx = static_cast<int>(x * invBinW);
            if (binIdx >= 256) {
                binIdx = 255;
            }

            int binH = static_cast<long>(histGlobal[0][binIdx]) * histH * binScale;

            for (int y = 0; y < histH; y++) {
                int binIdy = histH - 1 - y;
                uchar* pixel = histImg.ptr<uchar>(y, x);

                if (binIdy < binH) {
                    pixel[0] = 255;
                    pixel[1] = 255;
                    pixel[2] = 255;
                    pixel[3] = 255;
                }
            }
        }
        return {histImg, histImg};
    }

    // RGB image
    else {
        cv::Mat histLumImg(histH, histW, CV_8UC4, cv::Scalar(0, 0, 0, 255));

        // search the max value for normalization

        int maxB = *std::max_element(std::begin(histGlobal[0]), std::end(histGlobal[0]));
        int maxG = *std::max_element(std::begin(histGlobal[1]), std::end(histGlobal[1]));
        int maxR = *std::max_element(std::begin(histGlobal[2]), std::end(histGlobal[2]));
        int maxLum = *std::max_element(std::begin(histGlobal[3]), std::end(histGlobal[3]));

        int maxColor = std::max({maxB, maxG, maxR, maxLum});

        // Scale factor
        float scaleColor = (float)histH / maxColor;  // for color hist
        float scaleLum = (float)histH / maxLum;      // for luminance hist

        // Draw
        float invBinW = 1.0f / binW;

        // clang-format off
        #pragma omp parallel for
        // clang-format on
        for (int x = 0; x < histW; x++) {
            int binIdx = static_cast<int>(x * invBinW);

            if (binIdx >= 256) {
                binIdx = 255;
            }

            // calculate the bins heights
            int hB = static_cast<int>(histGlobal[0][binIdx] * scaleColor);
            int hG = static_cast<int>(histGlobal[1][binIdx] * scaleColor);
            int hR = static_cast<int>(histGlobal[2][binIdx] * scaleColor);
            int hL = static_cast<int>(histGlobal[3][binIdx] * scaleColor);

            // for luminance hist
            int hLum = static_cast<int>(histGlobal[3][binIdx] * scaleLum);

            for (int y = 0; y < histH; y++) {
                int invY = histH - 1 - y;

                uchar* colPtr = histImg.ptr<uchar>(y, x);
                uchar* lumPtr = histLumImg.ptr<uchar>(y, x);

                // color hist

                if (invY < hB) {
                    colPtr[0] = 255;
                    colPtr[1] = 0;
                    colPtr[2] = 0;
                    colPtr[3] = 128;
                }

                if (invY < hG) {
                    colPtr[0] = 0;
                    colPtr[1] = 255;
                    colPtr[2] = 0;
                    colPtr[3] = 128;
                }

                if (invY < hR) {
                    colPtr[0] = 0;
                    colPtr[1] = 0;
                    colPtr[2] = 255;
                    colPtr[3] = 128;
                }

                if (invY < hL) {
                    colPtr[0] = 255;
                    colPtr[1] = 255;
                    colPtr[2] = 255;
                    colPtr[3] = 255;
                }

                // luminance hist
                if (invY < hLum) {
                    lumPtr[0] = 255;
                    lumPtr[1] = 255;
                    lumPtr[2] = 255;
                    lumPtr[3] = 255;
                }
            }
        }
        return {histImg, histLumImg};
    }
}

//============================ Naive Implementation ========================

// std::tuple<cv::Mat, cv::Mat> Histogram::histogramImg(const cv::Mat& src) {
//     auto start = std::chrono::high_resolution_clock::now();
//     if (src.empty()) {
//         std::cerr << "Error in Histogram: loading image failed\n";
//         return {cv::Mat(), cv::Mat()};
//     }

//     // convert 16bit to 8
//     cv::Mat img8;
//     if (src.type() == CV_16UC1) {
//         // 255.0 / 65535.0 = 0.00389105 to avoid division
//         src.convertTo(img8, CV_8UC1, 0.00389105);
//     } else if (src.type() == CV_16UC3) {
//         src.convertTo(img8, CV_8UC3, 0.00389105);
//     } else {
//         img8 = src;
//     }

//     // hist dimensions
//     const int histSize = 256;
//     const int binW = 3;                 // Width of each bin
//     const int histW = histSize * binW;  // Width of the hist
//     const int histH = 2 * histW / 3;    // Heigh of the hist

//     // hist images
//     cv::Mat histImg(histH, histW, CV_8UC4, cv::Scalar(0, 0, 0, 0));

//     if (img8.type() == CV_8UC1) {
//         std::vector<int> hist(histSize, 0);

//         // iteration through the image to caculate the hist array
//         for (int y = 0; y < img8.rows; y++) {
//             const uchar* img8Ptr = img8.ptr<uchar>(y);

//             for (int x = 0; x < img8.cols; x++) {
//                 hist[img8Ptr[x]]++;
//             }
//         }

//         int maxVal = *std::max_element(hist.begin(), hist.end());
//         for (int i = 0; i < histSize; i++) {
//             int binH = hist[i] * histH / maxVal;
//             cv::Point tl = cv::Point(binW * i, histH - binH - 1);
//             cv::Point br = cv::Point(binW * (i + 1), histH - 1);

//             cv::rectangle(histImg, tl, br, cv::Scalar(255, 255, 255, 255), cv::FILLED);
//         }

//         return {histImg, histImg};
//     } else if (img8.type() == CV_8UC3) {
//         std::vector<int> histR(histSize, 0);
//         std::vector<int> histG(histSize, 0);
//         std::vector<int> histB(histSize, 0);
//         std::vector<int> histLum(histSize, 0);

//         cv::Mat histColor(histH, histW, CV_8UC4, cv::Scalar(0, 0, 0, 0));
//         cv::Mat histLumImg(histH, histW, CV_8UC4, cv::Scalar(0, 0, 0, 0));

//         for (int y = 0; y < img8.rows; y++) {
//             const cv::Vec3b* img8Ptr = img8.ptr<cv::Vec3b>(y);

//             for (int x = 0; x < img8.cols; x++) {
//                 int B = img8Ptr[x][0];
//                 int G = img8Ptr[x][1];
//                 int R = img8Ptr[x][2];

//                 int Y = (R * 77 + G * 150 + B * 29) >> 8;  // Bitshift for better performance

//                 histR[R]++;
//                 histG[G]++;
//                 histB[B]++;

//                 histLum[Y]++;
//             }
//         }

//         int maxR = *std::max_element(histR.begin(), histR.end());
//         int maxG = *std::max_element(histG.begin(), histG.end());
//         int maxB = *std::max_element(histB.begin(), histB.end());
//         int maxLum = *std::max_element(histLum.begin(), histLum.end());

//         int maxColor = std::max({maxR, maxG, maxB, maxLum});

//         for (int i = 0; i < histSize; i++) {
//             int binH = histR[i] * histH / maxColor;
//             cv::Point tl = cv::Point(i * binW, histH - binH - 1);
//             cv::Point br = cv::Point((i + 1) * binW, histH - 1);
//             cv::Scalar red = cv::Scalar(0, 0, 255, 255);
//             cv::rectangle(histColor, tl, br, red, cv::FILLED);
//         }

//         for (int i = 0; i < histSize; i++) {
//             int binH = histG[i] * histH / maxColor;
//             cv::Point tl = cv::Point(i * binW, histH - binH - 1);
//             cv::Point br = cv::Point((i + 1) * binW, histH - 1);
//             cv::Scalar green = cv::Scalar(0, 255, 0, 128);
//             cv::rectangle(histColor, tl, br, green, cv::FILLED);
//         }

//         for (int i = 0; i < histSize; i++) {
//             int binH = histB[i] * histH / maxColor;
//             cv::Point tl = cv::Point(i * binW, histH - binH - 1);
//             cv::Point br = cv::Point((i + 1) * binW, histH - 1);
//             cv::Scalar blue = cv::Scalar(255, 0, 0, 128);
//             cv::rectangle(histColor, tl, br, blue, cv::FILLED);
//         }

//         for (int i = 0; i < histSize; i++) {
//             int binH = histLum[i] * histH / maxColor;
//             cv::Point tl = cv::Point(i * binW, histH - binH - 1);
//             cv::Point br = cv::Point((i + 1) * binW, histH - 1);
//             cv::Scalar white = cv::Scalar(255, 255, 255, 255);
//             cv::rectangle(histColor, tl, br, white, cv::FILLED);
//         }

//         for (int i = 0; i < histSize; i++) {
//             int binH = histLum[i] * histH / maxLum;
//             cv::Point tl = cv::Point(i * binW, histH - binH - 1);
//             cv::Point br = cv::Point((i + 1) * binW, histH - 1);
//             cv::Scalar white = cv::Scalar(255, 255, 255, 255);
//             cv::rectangle(histLumImg, tl, br, white, cv::FILLED);
//         }
//         auto end = std::chrono::high_resolution_clock::now();
//         auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
//         std::cout << duration.count() << std::endl;
//         return {histColor, histLumImg};
//     } else {
//         return {cv::Mat(), cv::Mat()};
//     }
// }