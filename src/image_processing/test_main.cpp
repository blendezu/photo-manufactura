#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include "image_processing.h"

int main() {
    cv::Mat img = cv::imread("/Users/duongtran/Documents/testBilder/meinZoo.jpg");
    if (img.empty()) {
        std::cout << "Could not load image!" << std::endl;
        return -1;
    }

    ImageProcessor IP;

    cv::Mat croppedImg = IP.cropImg(img, cv::Rect(500, 500, 3000, 2500));

    std::cout << img.size() << " 1 \n";

    cv::imshow("croppedImg", croppedImg);
    cv::waitKey(0);

    return 0;
}