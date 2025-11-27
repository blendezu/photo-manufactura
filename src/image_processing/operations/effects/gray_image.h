#include <opencv2/core/hal/interface.h>

#include <opencv2/core/mat.hpp>

#include "operation_base.h"

class GrayImage : public ImageOperation {
   public:
    GrayImage() {};

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Monochrome";
    }

    std::string getSettings() const override {
        return "";
    }

   private:
    template <typename T, typename D>  // T: Vector type, D: data type of a value uchar / ushort
    cv::Mat grayImgTemplate(const cv::Mat& srcImg) {
        // Vector type for output image
        int dstType = (srcImg.depth() == CV_8U) ? CV_8UC1 : CV_16UC1;

        // output image
        cv::Mat dstImg(srcImg.size(), dstType);

        // clang-format off
        #pragma omp parallel for
        // clang-format on

        for (int y = 0; y < srcImg.rows; y++) {
            const T* srcPtr = srcImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                int B = srcPtr[x][0];
                int G = srcPtr[x][1];
                int R = srcPtr[x][2];

                int newVal = (54 * R + 183 * G + 18 * B) >> 8;
                dstImg.at<D>(y, x) = newVal;
            }
        }
        return dstImg;
    }
};