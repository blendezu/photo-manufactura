#include "histogram.h"

std::tuple<cv::Mat, cv::Mat> Histogram::histogramImg(const cv::Mat& src) {
    if (src.empty()) {
        std::cerr << "Error in Histogram: loading image failed\n";
        return {cv::Mat(), cv::Mat()};
    }

    // convert 16bit to 8
    cv::Mat img8;
    if (src.type() == CV_16UC1) {
        src.convertTo(img8, CV_8UC1, 255.0 / 65535.0);
    } else if (src.type() == CV_16UC3) {
        src.convertTo(img8, CV_8UC3, 255.0 / 65535.0);
    } else {
        img8 = src;
    }

    // hist dimensions
    int histSize = 256;
    int binW = 3;                 // Width of each bin
    int histW = histSize * binW;  // Width of the hist
    int histH = 2 * histW / 3;    // Heigh of the hist
    cv::Mat histImg(histH, histW, CV_8UC4, cv::Scalar(0, 0, 0, 0));

    if (img8.type() == CV_8UC1) {
        std::vector<int> hist(histSize, 0);

        // iteration through the image to caculate the hist array
        for (int y = 0; y < img8.rows; y++) {
            const uchar* img8Ptr = img8.ptr<uchar>(y);

            for (int x = 0; x < img8.cols; x++) {
                hist[img8Ptr[x]]++;
            }
        }

        int maxVal = *std::max_element(hist.begin(), hist.end());
        for (int i = 0; i < histSize; i++) {
            int binH = hist[i] * histH / maxVal;
            cv::Point tl = cv::Point(binW * i, histH - binH - 1);
            cv::Point br = cv::Point(binW * (i + 1), histH - 1);

            cv::rectangle(histImg, tl, br, cv::Scalar(255, 255, 255, 255), cv::FILLED);
        }

        return {histImg, histImg};
    } else if (img8.type() == CV_8UC3) {
        std::vector<int> histR(histSize, 0);
        std::vector<int> histG(histSize, 0);
        std::vector<int> histB(histSize, 0);
        std::vector<int> histLum(histSize, 0);

        cv::Mat histColor(histH, histW, CV_8UC4, cv::Scalar(0, 0, 0, 0));
        cv::Mat histLumImg(histH, histW, CV_8UC4, cv::Scalar(0, 0, 0, 0));

        for (int y = 0; y < img8.rows; y++) {
            const cv::Vec3b* img8Ptr = img8.ptr<cv::Vec3b>(y);

            for (int x = 0; x < img8.cols; x++) {
                int B = img8Ptr[x][0];
                int G = img8Ptr[x][1];
                int R = img8Ptr[x][2];

                int Y = 0.299f * R + 0.587f * G + 0.114f * B;

                histR[R]++;
                histG[G]++;
                histB[B]++;

                histLum[Y]++;
            }
        }

        int maxR = *std::max_element(histR.begin(), histR.end());
        int maxG = *std::max_element(histG.begin(), histG.end());
        int maxB = *std::max_element(histB.begin(), histB.end());
        int maxLum = *std::max_element(histLum.begin(), histLum.end());

        int maxColor = std::max({maxR, maxG, maxB, maxLum});

        for (int i = 0; i < histSize; i++) {
            int binH = histR[i] * histH / maxColor;
            cv::Point tl = cv::Point(i * binW, histH - binH - 1);
            cv::Point br = cv::Point((i + 1) * binW, histH - 1);
            cv::Scalar red = cv::Scalar(0, 0, 255, 255);
            cv::rectangle(histColor, tl, br, red, cv::FILLED);
        }

        for (int i = 0; i < histSize; i++) {
            int binH = histG[i] * histH / maxColor;
            cv::Point tl = cv::Point(i * binW, histH - binH - 1);
            cv::Point br = cv::Point((i + 1) * binW, histH - 1);
            cv::Scalar green = cv::Scalar(0, 255, 0, 128);
            cv::rectangle(histColor, tl, br, green, cv::FILLED);
        }

        for (int i = 0; i < histSize; i++) {
            int binH = histB[i] * histH / maxColor;
            cv::Point tl = cv::Point(i * binW, histH - binH - 1);
            cv::Point br = cv::Point((i + 1) * binW, histH - 1);
            cv::Scalar blue = cv::Scalar(255, 0, 0, 128);
            cv::rectangle(histColor, tl, br, blue, cv::FILLED);
        }

        for (int i = 0; i < histSize; i++) {
            int binH = histLum[i] * histH / maxColor;
            cv::Point tl = cv::Point(i * binW, histH - binH - 1);
            cv::Point br = cv::Point((i + 1) * binW, histH - 1);
            cv::Scalar white = cv::Scalar(255, 255, 255, 255);
            cv::rectangle(histColor, tl, br, white, cv::FILLED);
        }

        for (int i = 0; i < histSize; i++) {
            int binH = histLum[i] * histH / maxLum;
            cv::Point tl = cv::Point(i * binW, histH - binH - 1);
            cv::Point br = cv::Point((i + 1) * binW, histH - 1);
            cv::Scalar white = cv::Scalar(255, 255, 255, 255);
            cv::rectangle(histLumImg, tl, br, white, cv::FILLED);
        }

        return {histColor, histLumImg};
    } else {
        return {cv::Mat(), cv::Mat()};
    }
}