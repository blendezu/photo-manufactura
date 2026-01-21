#include <opencv2/core/mat.hpp>

#include "operation_base.h"

class Vintage1 : public ImageOperation {
   private:
    static cv::Mat scratchImg;
    static cv::Mat cachedScratchMask;
    static cv::Size lastSize;

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