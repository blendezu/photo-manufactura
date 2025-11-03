#pragma one
#include "../core/operation_base.h"

class Flip : ImageOperation {
   private:
    int flipDir;

   public:
    Flip(int flipDirdirection) : flipDir(flipDirdirection) {}

    std::string getName() {
        return "Flip";
    }

    cv::Mat apply(const cv::Mat& scrImg) override;

    void setDirection(int direction) {
        flipDir = direction;
    }
    int getDirection() {
        return flipDir;
    }

   private:
    /**
     * @brief Flip image along horizontal or vertical axis
     * @param iPut Input source image
     * @param flipCode 0 for vertical (↕) flipping, 1 for horizontal (↔)
     * @return Flipped image
     */
    template <typename T>
    static cv::Mat flipImgTemplate(const cv::Mat& iPut, int flipCode) {
        // check if the input image is empty
        if (iPut.empty()) {
            std::cerr << "Error in flipImgTemplate: the input image is empty\n";
            return cv::Mat();
        }
        const int imgW = iPut.cols;
        const int imgH = iPut.rows;
        const int channels = iPut.channels();

        cv::Mat flippedImg(iPut.size(), iPut.type());

        const bool horizontal = (flipCode == 1);

        // --- Horizontal flip (left <-> right) ---
        if (horizontal) {
            for (int y = 0; y < imgH; y++) {
                const T* iPtr = iPut.ptr<T>(y);
                T* oPtr = flippedImg.ptr<T>(y);

                for (int x = 0; x < imgW; x++) {
                    oPtr[imgW - 1 - x] = iPtr[x];
                }
            }
        }
        // --- Vertical flip (top ↕ bottom) ---
        else {
            for (int y = 0; y < imgH; y++) {
                const T* iPtr = iPut.ptr<T>(y);
                T* oPtr = flippedImg.ptr<T>(imgH - 1 - y);

                std::memcpy(oPtr, iPtr, imgW * sizeof(T));
            }
        }
        return flippedImg;
    }
};