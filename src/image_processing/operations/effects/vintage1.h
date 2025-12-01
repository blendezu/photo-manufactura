#include <opencv2/core/mat.hpp>

#include "operation_base.h"

class Vintage1 : public ImageOperation {
   private:
    cv::Mat scratchImg = cv::imread("images/9003.jpg");

   public:
    Vintage1() {};

    cv::Mat apply(const cv::Mat& srcImg) override;

    std::string getName() const override {
        return "Vintage 1";
    }

    std::string getSettings() const override {
        return "";
    }
};