#include "operation_base.h"

class AdjustContrast : public ImageOperation {
   private:
    int contrast;

   public:
    AdjustContrast(int value) : contrast(value) {}

    cv::Mat apply(const cv::Mat& srcImg);

    std::string getSettings() {
        return
    }
};