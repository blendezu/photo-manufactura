#ifndef ROTATE_H
#define ROTATE_H
#include <opencv2/core/types.hpp>

#include "image_utils.h"
#include "operation_base.h"

class Rotate : public ImageOperation {
   private:
    int angle_deg = 0;  // angle in degree
    cv::Rect roi;

   public:
    Rotate(int angle_deg, cv::Rect roi = cv::Rect()) : angle_deg(angle_deg), roi(roi) {}

    std::string getName() const override {
        return "Rotate";
    }

    cv::Mat apply(const cv::Mat& srcImg) override;

    void setAngle(int angle) {
        angle_deg = angle;
    }
    int getAngle() const {
        return angle_deg;
    }

    void setROI(cv::Rect rect) {
        roi = rect;
    }
    cv::Rect getROI() const {
        return roi;
    }

   private:
    /**
     * @brief Rotate image by specified angle around center
     * @brief Interpolation method: bicubic
     * @param srcImg Input source image
     * @param angle Rotation angle in degrees (positive = counter-clockwise)
     * @return Rotated image with preserved content
     */
    template <typename T>
    static cv::Mat rotateImgTemplate(const cv::Mat& srcImg, int angle_deg, cv::Rect roi) {
        if (srcImg.empty()) {
            std::cerr << "Error: input image is empty\n";
            return cv::Mat();
        }

        const int imgW = srcImg.cols;
        const int imgH = srcImg.rows;

        // ROI validity check
        if (roi.x < 0 || roi.y < 0 || roi.x + roi.width > imgW || roi.y + roi.height > imgH) {
            std::cerr << "Error in rotateImgTemplate: ROI is not valid\n";
            return cv::Mat();
        }

        cv::Mat dstImg(srcImg.size(), srcImg.type(), cv::Scalar(0));  // output image
        const int cx = imgW / 2;
        const int cy = imgH / 2;

        // for the transformation atrix
        const double angle_rad = angle_deg * M_PI / 180.0;
        const double cosA = std::cos(angle_rad);
        const double sinA = std::sin(angle_rad);

        // Line pointers for better access
        std::vector<const T*> srcRowPtrs(imgH);
        for (int y = 0; y < imgH; y++) {
            srcRowPtrs[y] = srcImg.ptr<T>(y);
        }

        // Boundary-Check
        auto inBounds = [imgW, imgH](int x, int y) -> bool {
            return static_cast<unsigned>(x) < static_cast<unsigned>(imgW) &&
                   static_cast<unsigned>(y) < static_cast<unsigned>(imgH);
        };

        // clang-format off
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < imgH; y++) {
            T* outPtr = dstImg.ptr<T>(y);

            for (int x = 0; x < imgW; x++) {
                // Inverse Mapping
                float dx = x - cx;
                float dy = y - cy;
                float oldX = dx * cosA + dy * sinA + cx;
                float oldY = -dx * sinA + dy * cosA + cy;

                int i = static_cast<int>(oldX);
                int j = static_cast<int>(oldY);
                float a = oldX - i;
                float b = oldY - j;

                // Gewichte vorberechnen
                float wx[4], wy[4];
                for (int m = 0; m < 4; m++) {
                    wx[m] = ImageUtils::calculateCubicWeight(a - (m - 1));
                }
                for (int n = 0; n < 4; n++) {
                    wy[n] = ImageUtils::calculateCubicWeight(b - (n - 1));
                }

                // Pixel-Interpolation
                if constexpr (std::is_same_v<T, uchar> || std::is_same_v<T, uint16_t>) {
                    float sum = 0.0;
                    float weightSum = 0.0;

                    for (int n = 0; n < 4; n++) {
                        int yj = j + n - 1;
                        if (!inBounds(0, yj))
                            continue;

                        const T* srcPtr = srcRowPtrs[yj];
                        for (int m = 0; m < 4; m++) {
                            int xi = i + m - 1;
                            if (!inBounds(xi, 0))
                                continue;

                            float weight = wx[m] * wy[n];
                            sum += static_cast<float>(srcPtr[xi]) * weight;
                            weightSum += weight;
                        }
                    }

                    // Normalisierung um Artefakte zu reduzieren
                    if (weightSum > 1e-6) {
                        sum /= weightSum;
                    }

                    if constexpr (std::is_same_v<T, uchar>) {
                        outPtr[x] = static_cast<uchar>(std::clamp(sum, 0.0f, 255.0f));
                    } else {
                        outPtr[x] = static_cast<uint16_t>(std::clamp(sum, 0.0f, 65535.0f));
                    }

                } else if constexpr (std::is_same_v<T, cv::Vec3b> || std::is_same_v<T, cv::Vec3w>) {
                    float sum[3] = {0, 0, 0};
                    float weightSum = 0.0;

                    for (int n = 0; n < 4; n++) {
                        int yj = j + n - 1;
                        if (!inBounds(0, yj))
                            continue;

                        const T* srcPtr = srcRowPtrs[yj];
                        for (int m = 0; m < 4; m++) {
                            int xi = i + m - 1;
                            if (!inBounds(xi, 0))
                                continue;

                            float weight = wx[m] * wy[n];
                            const auto& pixel = srcPtr[xi];

                            for (int c = 0; c < 3; c++) {
                                sum[c] += static_cast<float>(pixel[c]) * weight;
                            }
                            weightSum += weight;
                        }
                    }

                    // Normalisierung
                    if (weightSum > 1e-6) {
                        for (int c = 0; c < 3; c++) {
                            sum[c] /= weightSum;
                        }
                    }

                    if constexpr (std::is_same_v<T, cv::Vec3b>) {
                        for (int c = 0; c < 3; c++) {
                            outPtr[x][c] = static_cast<uchar>(std::clamp(sum[c], 0.0f, 255.0f));
                        }
                    } else {
                        for (int c = 0; c < 3; c++) {
                            outPtr[x][c] =
                                static_cast<uint16_t>(std::clamp(sum[c], 0.0f, 65535.0f));
                        }
                    }
                }
            }
        }
        return dstImg(roi).clone();
    }
};

#endif  // ROTATE_H