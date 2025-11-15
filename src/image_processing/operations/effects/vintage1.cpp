#include "vintage1.h"

#include <opencv2/core/hal/interface.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include "image_resize.h"
#include "image_utils.h"

cv::Mat Vintage1::apply(const cv::Mat& srcImg) {
    if (srcImg.empty()) {
        std::cerr << "Error in Vintage1: empty input image\n";
        return cv::Mat();
    }

    cv::Mat scratchImg = cv::imread("images/9003.jpg");
    cv::Mat satReducedImg = ImageUtils::setSaturationTo(srcImg, 0.0f);
    cv::Mat blendedImg = ImageUtils::blendScratch(satReducedImg, scratchImg);
    if (srcImg.channels() == 3) {
        cv::Mat warmImg = ImageUtils::setVintageWarm(blendedImg);
        return warmImg;
    } else {
        return blendedImg;
    }
}