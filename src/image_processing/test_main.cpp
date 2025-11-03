#include <iostream>
#include <opencv2/opencv.hpp>

int main() {
    cv::Mat img = cv::imread("/Users/duongtran/Documents/testBilder/meinZoo.jpg");
    if (img.empty()) {
        std::cerr << "The image is empty\n";
    }

    cv::imshow("img", img);
    cv::waitKey(0);

    return 0;
}