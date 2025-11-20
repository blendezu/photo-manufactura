#include <opencv2/core/hal/interface.h>

#include <algorithm>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/core/saturate.hpp>
#include <string>

#include "image_utils.h"
#include "operation_base.h"

using uint = unsigned int;

class ResizeImage : public ImageOperation {
   private:
    uint imgH = 0;
    uint imgW = 0;
    double ratio = 0;

   public:
    ResizeImage(uint h, uint w) : imgH(h), imgW(w) {}

    ResizeImage(uint h, double r) : imgH(h), ratio(r) {
        imgW = imgH * ratio;
    }

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Resize";
    }

    std::string getSettings() const override {
        return "width: " + std::to_string(imgW) + "; height: " + std::to_string(imgH) +
               "; ration: " + std::to_string(ratio);
    }

    void setHeight(uint h) {
        imgH = h;
        if (ratio != 0) {
            imgW = h * ratio;
        }
    }

    uint getHeight() {
        return imgH;
    }

    void setWidth(uint w) {
        if (ratio == 0) {
            imgW = w;
        } else {
        }
    }

    uint getWidth() {
        return imgW;
    }

    void setRatio(double r) {
        if (r > 0) {
            ratio = r;
            imgW = imgH * ratio;
        }
    }

    double getRation() {
        return ratio;
    }

   private:
    template <typename T>
    cv::Mat resizeGrayImgTemplate(const cv::Mat& srcImg) {
        double scaleFactorX = (float)imgW / srcImg.cols;
        double scaleFactorY = (float)imgH / srcImg.rows;

        cv::Mat dstImg(imgH, imgW, srcImg.type());

        auto findOldX = [scaleFactorX](int newX) { return newX / scaleFactorX; };
        auto findOldY = [scaleFactorY](int newY) { return newY / scaleFactorY; };

        for (int y = 0; y < dstImg.rows; y++) {
            for (int x = 0; x < dstImg.cols; x++) {
                double newPixel = 0.0;

                double oldX = findOldX(x);
                double oldY = findOldY(y);

                double oXD = std::floor(oldX);
                double oYD = std::floor(oldY);

                for (int i = -1; i < 3; i++) {
                    int Y = oYD + i;
                    Y = std::clamp(Y, 0, srcImg.rows - 1);

                    double tY = oldY - (oYD + i);
                    double wY = ImageUtils::calculateCubicWeight(tY);

                    for (int j = -1; j < 3; j++) {
                        int X = oXD + j;
                        X = std::clamp(X, 0, srcImg.cols - 1);

                        double tX = oldX - (oXD + j);
                        double wX = ImageUtils::calculateCubicWeight(tX);

                        newPixel += wX * wY * srcImg.at<T>(Y, X);
                    }
                }
                double maxRange = 0.0;
                (srcImg.depth() == CV_16U) ? maxRange = 65535.0 : maxRange = 255.0;
                dstImg.at<T>(y, x) = static_cast<T>(std::clamp(newPixel, 0.0, maxRange));
            }
        }
        return dstImg;
    }

    template <typename T>
    cv::Mat resizeBGRImgTemplate(const cv::Mat& srcImg) {
        auto start = std::chrono::high_resolution_clock::now();

        cv::Mat dstImg(imgH, imgW, srcImg.type());  // resized image

        double scaleFactorX = (double)imgW / srcImg.cols;
        double scaleFactorY = (double)imgH / srcImg.rows;

        for (int y = 0; y < dstImg.rows; y++) {
            T* dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < dstImg.cols; x++) {
                double oldX = x / scaleFactorX;
                double oldY = y / scaleFactorY;

                double newR = 0.0;
                double newG = 0.0;
                double newB = 0.0;

                double oX = std::floor(oldX);
                double oY = std::floor(oldY);

                for (int i = -1; i < 3; i++) {
                    double tY = (oY + i) - oldY;
                    double wY = ImageUtils::calculateCubicWeight(tY);

                    int Y = oY + i;
                    Y = std::clamp(Y, 0, srcImg.rows - 1);

                    for (int j = -1; j < 3; j++) {
                        double tX = (oX + j) - oldX;
                        double wX = ImageUtils::calculateCubicWeight(tX);

                        int X = static_cast<int>(oX + j);
                        X = std::clamp(X, 0, srcImg.cols - 1);

                        T pixel = srcImg.at<T>(Y, X);
                        newB += wX * wY * pixel[0];
                        newG += wX * wY * pixel[1];
                        newR += wX * wY * pixel[2];
                    }
                }
                if (srcImg.depth() == CV_8U) {
                    newB = cv::saturate_cast<uchar>(newB);
                    newG = cv::saturate_cast<uchar>(newG);
                    newR = cv::saturate_cast<uchar>(newR);
                } else {
                    newB = cv::saturate_cast<ushort>(newB);
                    newG = cv::saturate_cast<ushort>(newG);
                    newR = cv::saturate_cast<ushort>(newR);
                }

                dstPtr[x] = T(newB, newG, newR);
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        // std::cout << duration.count() << std::endl;
        return dstImg;
    }
};