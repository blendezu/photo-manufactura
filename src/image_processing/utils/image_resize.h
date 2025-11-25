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
        float scaleFactorX = (float)imgW / srcImg.cols;
        float scaleFactorY = (float)imgH / srcImg.rows;

        cv::Mat dstImg(imgH, imgW, srcImg.type());

        auto findOldX = [scaleFactorX](int newX) { return newX / scaleFactorX; };
        auto findOldY = [scaleFactorY](int newY) { return newY / scaleFactorY; };

        for (int y = 0; y < dstImg.rows; y++) {
            for (int x = 0; x < dstImg.cols; x++) {
                float newPixel = 0.0f;

                float oldX = findOldX(x);
                float oldY = findOldY(y);

                float oXD = std::floor(oldX);
                float oYD = std::floor(oldY);

                for (int i = -1; i < 3; i++) {
                    int Y = oYD + i;
                    Y = std::clamp(Y, 0, srcImg.rows - 1);

                    float tY = oldY - (oYD + i);
                    float wY = ImageUtils::calculateCubicWeight(tY);

                    for (int j = -1; j < 3; j++) {
                        int X = oXD + j;
                        X = std::clamp(X, 0, srcImg.cols - 1);

                        float tX = oldX - (oXD + j);
                        float wX = ImageUtils::calculateCubicWeight(tX);

                        newPixel += wX * wY * srcImg.at<T>(Y, X);
                    }
                }
                float maxRange = 0.0f;
                (srcImg.depth() == CV_16U) ? maxRange = 65535.0f : maxRange = 255.0f;
                dstImg.at<T>(y, x) = static_cast<T>(std::clamp(newPixel, 0.0f, maxRange));
            }
        }
        return dstImg;
    }

    template <typename T>
    cv::Mat resizeBGRImgTemplate(const cv::Mat& srcImg) {
        cv::Mat dstImg(imgH, imgW, srcImg.type());  // resized image

        // inversed scale to avoid division in the for loop
        float scaleFactorX = static_cast<float>(srcImg.cols) / imgW;
        float scaleFactorY = static_cast<float>(srcImg.rows) / imgH;

        // clang-format off
        #pragma omp parallel for
        // clang-format on

        for (int y = 0; y < dstImg.rows; y++) {
            T* dstPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < dstImg.cols; x++) {
                float oldX = x * scaleFactorX;
                float oldY = y * scaleFactorY;

                float newR = 0.0f;
                float newG = 0.0f;
                float newB = 0.0f;

                float oX = std::floor(oldX);
                float oY = std::floor(oldY);

                for (int i = -1; i < 3; i++) {
                    float tY = (oY + i) - oldY;
                    float wY = ImageUtils::calculateCubicWeight(tY);

                    int Y = oY + i;
                    Y = std::clamp(Y, 0, srcImg.rows - 1);

                    for (int j = -1; j < 3; j++) {
                        float tX = (oX + j) - oldX;
                        float wX = ImageUtils::calculateCubicWeight(tX);

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
        return dstImg;
    }
};