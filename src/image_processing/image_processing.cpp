#include "image_processing.h"

#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

// =========================================================================
// GEOMETRIC TRANSFORMATIONS
// =========================================================================

cv::Mat ImageProcessor::cropImg(const cv::Mat& iPut, const cv::Rect& roi) {
    // check if the image is empty
    if (iPut.empty()) {
        std::cerr << "Error in cropImg: the input image is empty\n";
        return cv::Mat();
    }

    // check if the roi area is valid
    bool inside = roi.x >= 0 && roi.y >= 0 && roi.x + roi.width <= iPut.cols &&
                  roi.y + roi.height <= iPut.rows;

    if (inside) {
        return iPut(roi).clone();
    } else {
        std::cerr << "Error in cropImg: the roi area is not valid\n";
        return cv::Mat();
    }
    // another way to check it
    // cv::Rect imgRect = cv::Rect(0, 0, iPut.cols, iPut.rows);
    // cv::Rect validRect = roi & imgRect; // overlapping area
    // if(validRect.empty()) {

    // }
}

// naiv Implementation --> bad performance: 244287 micro seconds
// cv::Mat ImageProcessor::rotateImg1(const cv::Mat& iPut, int angle_deg, cv::Rect roi) {

//     auto start = std::chrono::high_resolution_clock::now();
//     if (iPut.empty()) {
//         std::cerr << "Error in rotateImg(): the input image is empty\n";
//         return cv::Mat();
//     }

//     int imgW = iPut.cols;
//     int imgH = iPut.rows;

//     // check if roi is valid
//     cv::Rect imgRect(0, 0, imgW, imgH);
//     if ((imgRect & roi).empty()) {
//         std::cerr << "Error in rotateImg(): the roi area is not valid\n";
//         return iPut;
//     }

//     cv::Mat rotatedImg(iPut.size(), iPut.type(), cv::Scalar(0, 0, 0)); // fill with black

//     // rotation point (center)
//     int cx = imgW / 2;
//     int cy = imgH / 2;

//     double angle_rad = angle_deg * M_PI / 180.0;
//     double cosA = std::cos(angle_rad);
//     double sinA = std::sin(angle_rad);

//     auto cubicWeight = [](double t) -> double {
//         t = std::abs(t);
//         if      (t<1) { return 1.5*t*t*t - 2.5*t*t + 1; }
//         else if (t<2) { return -0.5*t*t*t + 2.5*t*t - 4*t + 2; }
//         else          { return 0.0; }
//     };

//     for (int y = 0; y < imgH; y++) {
//         cv::Vec3b* outPtr = rotatedImg.ptr<cv::Vec3b>(y);
//         for (int x = 0; x < imgW; x++) {

//             // Inverse Mapping
//             double oldX = (x - cx) *  cosA + (y - cy) * sinA + cx;
//             double oldY = (x - cx) * -sinA + (y - cy) * cosA + cy;

//             int i = static_cast<int>(std::floor(oldX));
//             int j = static_cast<int>(std::floor(oldY));
//             double a = oldX - i;
//             double b = oldY - j;

//             cv::Vec3d sum(0, 0, 0);

//             for (int m = -1; m <= 2; m++) {
//                 double wx = cubicWeight(a-m);
//                 int xi = i + m;

//                 for (int n = -1; n <= 2; n++) {
//                     double wy = cubicWeight(b - n);
//                     int yj = j + n;

//                     cv::Vec3b pixel(0, 0, 0); // black as default
//                     if (xi >= 0 && xi < imgW && yj >= 0 && yj < imgH) {
//                         pixel = iPut.at<cv::Vec3b>(yj, xi);
//                     }

//                     sum[0] += pixel[0] * wx * wy;
//                     sum[1] += pixel[1] * wx * wy;
//                     sum[2] += pixel[2] * wx * wy;
//                 }
//             }
//             // Clamp auf 0-255
//             outPtr[x][0] = std::min(255.0, std::max(0.0, sum[0]));
//             outPtr[x][1] = std::min(255.0, std::max(0.0, sum[1]));
//             outPtr[x][2] = std::min(255.0, std::max(0.0, sum[2]));
//         }
//     }
//     auto end = std::chrono::high_resolution_clock::now();
//     auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
//     std::cout << duration.count() << std::endl;
//     return rotatedImg(roi).clone();
// }

// efficient --> better performance: 164023 micro second
// cv::Mat ImageProcessor::rotateImg(const cv::Mat& iPut, int angle_deg, cv::Rect roi) {
//     if (iPut.empty()) {
//         std::cerr << "Error in rotateImg(): the input image is empty\n";
//         return cv::Mat();
//     }

//     int imgW = iPut.cols;
//     int imgH = iPut.rows;

//     cv::Rect imgRect(0, 0, imgW, imgH);
//     if ((imgRect & roi).empty()) {
//         std::cerr << "Error in rotateImg(): the roi area is not valid\n";
//         return iPut;
//     }

//     cv::Mat rotatedImg(iPut.size(), iPut.type(), cv::Scalar(0, 0, 0));

//     int cx = imgW / 2;
//     int cy = imgH / 2;

//     double angle_rad = angle_deg * M_PI / 180.0;
//     double cosA = std::cos(angle_rad);
//     double sinA = std::sin(angle_rad);

//     auto cubicWeight = [](double t) -> double {
//         t = std::abs(t);
//         if (t < 1) return 1.5*t*t*t - 2.5*t*t + 1;
//         else if (t < 2) return -0.5*t*t*t + 2.5*t*t - 4*t + 2;
//         else return 0.0;
//     };

//     // Iteration throug the image
//     for (int y = 0; y < imgH; y++) {
//         cv::Vec3b* outPtr = rotatedImg.ptr<cv::Vec3b>(y);

//         for (int x = 0; x < imgW; x++) {
//             // Inverse Mapping
//             double oldX =  (x - cx) * cosA + (y - cy) * sinA + cx;
//             double oldY = -(x - cx) * sinA + (y - cy) * cosA + cy;

//             int i = static_cast<int>(std::floor(oldX));
//             int j = static_cast<int>(std::floor(oldY));
//             double a = oldX - i;
//             double b = oldY - j;

//             // precaculate weights for X and Y
//             double wx[4], wy[4];
//             for (int m = 0; m < 4; m++) wx[m] = cubicWeight(a - (m - 1));
//             for (int n = 0; n < 4; n++) wy[n] = cubicWeight(b - (n - 1));

//             cv::Vec3d sum(0,0,0);

//             for (int n = 0; n < 4; n++) {
//                 int yj = j + n - 1;
//                 if (yj < 0 || yj >= imgH) continue; // outside -> black

//                 const cv::Vec3b* srcPtr = iPut.ptr<cv::Vec3b>(yj);
//                 for (int m = 0; m < 4; m++) {
//                     int xi = i + m - 1;
//                     if (xi < 0 || xi >= imgW) continue; // outside -> black

//                     cv::Vec3b pix = srcPtr[xi];
//                     double w = wx[m] * wy[n];
//                     sum[0] += pix[0] * w;
//                     sum[1] += pix[1] * w;
//                     sum[2] += pix[2] * w;
//                 }
//             }

//             outPtr[x][0] = static_cast<uchar>(std::min(255.0, std::max(0.0, sum[0])));
//             outPtr[x][1] = static_cast<uchar>(std::min(255.0, std::max(0.0, sum[1])));
//             outPtr[x][2] = static_cast<uchar>(std::min(255.0, std::max(0.0, sum[2])));
//         }
//     }
//     return rotatedImg(roi).clone();
// }

// Template for all data type of input image

cv::Mat ImageProcessor::Histogram(const cv::Mat& img) {
    if (img.empty()) {
        return cv::Mat();
    }

    // Feste Werte
    int bins = 256;
    int width = 512;
    int height = 3 * width / 2;

    cv::Mat img8;
    if (img.type() == CV_16UC1)
        img.convertTo(img8, CV_8UC1, 255.0 / 65535.0);
    else if (img.type() == CV_16UC3)
        img.convertTo(img8, CV_8UC3, 255.0 / 65535.0);
    else if (img.type() == CV_8UC1 || img.type() == CV_8UC3)
        img8 = img.clone();
    else
        throw std::runtime_error("Unsupported image type");

    std::vector<cv::Mat> channels;
    cv::split(img8, channels);

    cv::Mat histImage(height, width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    int bin_w = cvRound((double)width / bins);

    std::vector<cv::Mat> hists(channels.size());
    for (int c = 0; c < channels.size(); ++c) {
        cv::calcHist(&channels[c], 1, 0, cv::Mat(), hists[c], 1, &bins, 0, true, false);
        cv::normalize(hists[c], hists[c], 0, height, cv::NORM_MINMAX);
    }

    cv::Scalar colors[4] = {cv::Scalar(255, 0, 0, 255), cv::Scalar(0, 255, 0, 255),
                            cv::Scalar(0, 0, 255, 255)};

    for (int i = 1; i < bins; ++i) {
        for (int c = 0; c < channels.size(); ++c) {
            cv::line(histImage,
                     cv::Point(bin_w * (i - 1), height - cvRound(hists[c].at<float>(i - 1))),
                     cv::Point(bin_w * (i), height - cvRound(hists[c].at<float>(i))), colors[c % 3],
                     2, 8, 0);
        }
    }

    return histImage;
}
