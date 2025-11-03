#ifndef ROTATE_H
#define ROTATE_H
#include <opencv2/core/types.hpp>

#include "../core/operation_base.h"

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
     * @param iPut Input source image
     * @param angle Rotation angle in degrees (positive = counter-clockwise)
     * @return Rotated image with preserved content
     */
    template <typename T>
    static cv::Mat rotateImgTemplate(const cv::Mat& iPut, int angle_deg, cv::Rect roi) {
        if (iPut.empty()) {
            std::cerr << "Error: input image is empty\n";
            return cv::Mat();
        }

        int imgW = iPut.cols;
        int imgH = iPut.rows;

        // check ROI validity
        cv::Rect imgRect(0, 0, imgW, imgH);
        if ((imgRect & roi).empty()) {
            std::cerr << "Error in rotateImgTemplate: ROI is not valid\n";
            return cv::Mat();
        }

        cv::Mat rotatedImg(iPut.size(), iPut.type(),
                           cv::Scalar(0));  // black backgroun for black corner
        int cx = imgW / 2;
        int cy = imgH / 2;

        double angle_rad = angle_deg * M_PI / 180.0;
        double cosA = std::cos(angle_rad);
        double sinA = std::sin(angle_rad);

        auto cubicWeight = [](double t) -> double {
            t = std::abs(t);
            if (t < 1)
                return 1.5 * t * t * t - 2.5 * t * t + 1;
            else if (t < 2)
                return -0.5 * t * t * t + 2.5 * t * t - 4 * t + 2;
            else
                return 0.0;
        };

        int channels = iPut.channels();

        for (int y = 0; y < imgH; y++) {
            T* outPtr = rotatedImg.ptr<T>(y);
            for (int x = 0; x < imgW; x++) {
                double oldX = (x - cx) * cosA + (y - cy) * sinA + cx;
                double oldY = -(x - cx) * sinA + (y - cy) * cosA + cy;

                int i = static_cast<int>(std::floor(oldX));
                int j = static_cast<int>(std::floor(oldY));
                double a = oldX - i;
                double b = oldY - j;

                double wx[4], wy[4];
                for (int m = 0; m < 4; m++)
                    wx[m] = cubicWeight(a - (m - 1));
                for (int n = 0; n < 4; n++)
                    wy[n] = cubicWeight(b - (n - 1));

                std::vector<double> sum(channels, 0.0);

                for (int n = 0; n < 4; n++) {
                    int yj = j + n - 1;
                    if (yj < 0 || yj >= imgH)
                        continue;
                    const T* srcPtr = iPut.ptr<T>(yj);

                    for (int m = 0; m < 4; m++) {
                        int xi = i + m - 1;
                        if (xi < 0 || xi >= imgW)
                            continue;

                        if (channels == 1) {
                            if constexpr (std::is_same<T, uchar>::value) {
                                sum[0] += static_cast<double>(srcPtr[xi]) * wx[m] * wy[n];
                            } else if constexpr (std::is_same<T, uint16_t>::value) {
                                sum[0] += static_cast<double>(srcPtr[xi]) * wx[m] * wy[n];
                            }
                        } else if (channels == 3) {
                            auto pix = srcPtr[xi];
                            for (int c = 0; c < 3; c++) {
                                if constexpr (std::is_same<T, cv::Vec3b>::value) {
                                    sum[c] += static_cast<double>(pix[c]) * wx[m] * wy[n];
                                } else if constexpr (std::is_same<T, cv::Vec3w>::value) {
                                    sum[c] += static_cast<double>(pix[c]) * wx[m] * wy[n];
                                }
                            }
                        }
                    }
                }

                // write back
                if (channels == 1) {
                    if constexpr (std::is_same<T, uchar>::value) {
                        outPtr[x] = static_cast<uchar>(std::min(std::max(sum[0], 0.0), 255.0));
                    } else if constexpr (std::is_same<T, uint16_t>::value) {
                        outPtr[x] = static_cast<uint16_t>(std::min(std::max(sum[0], 0.0), 65535.0));
                    }
                } else if (channels == 3) {
                    for (int c = 0; c < 3; c++) {
                        if constexpr (std::is_same<T, cv::Vec3b>::value) {
                            outPtr[x][c] =
                                static_cast<uchar>(std::min(std::max(sum[c], 0.0), 255.0));
                        } else if constexpr (std::is_same<T, cv::Vec3w>::value) {
                            outPtr[x][c] =
                                static_cast<uint16_t>(std::min(std::max(sum[c], 0.0), 65535.0));
                        }
                    }
                }
            }
        }

        return rotatedImg(roi).clone();
    }
};

#endif  // ROTATE_H
