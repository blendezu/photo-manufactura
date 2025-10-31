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
}

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
