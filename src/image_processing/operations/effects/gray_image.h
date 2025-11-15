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
    template <typename T, typename D>
    cv::Mat grayImgTemplate(const cv::Mat& srcImg) {
        int dstType = (srcImg.depth() == CV_8U) ? CV_8UC1 : CV_16UC1;
        cv::Mat dstImg(srcImg.size(), dstType);

        for (int y = 0; y < srcImg.rows; y++) {
            const T* srcPtr = srcImg.ptr<T>(y);

            for (int x = 0; x < srcImg.cols; x++) {
                int B = srcPtr[x][0];
                int G = srcPtr[x][1];
                int R = srcPtr[x][2];

                float Val = 0.2125 * R + 0.7154 * G + 0.072 * B;
                dstImg.at<D>(y, x) = static_cast<D>(std::round(Val));
            }
        }
        return dstImg;
    }
};