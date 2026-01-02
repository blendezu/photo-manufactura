#ifndef ROTATE_H
#define ROTATE_H
#include <Halide.h>

#include <opencv2/core/types.hpp>

#include "image_utils.h"
#include "operation_base.h"

class Rotate : public HalideOperation {
   private:
    int m_angle_deg = 0;  // angle in degree
    cv::Rect m_roi;

    // --- Halide Runtime Parameters ---
    Halide::Param<float> p_cosA{"rotate_cosA"};
    Halide::Param<float> p_sinA{"rotate_sinA"};
    Halide::Param<int> p_cx{"rotate_cx"};
    Halide::Param<int> p_cy{"rotate_cy"};
    Halide::Param<int> p_src_width{"rotate_src_width"};
    Halide::Param<int> p_src_height{"rotate_src_height"};
    Halide::Param<int> p_roi_x{"rotate_roi_x"};
    Halide::Param<int> p_roi_y{"rotate_roi_y"};

   public:
    Rotate(int angle_deg, cv::Rect roi = cv::Rect()) : m_angle_deg(angle_deg), m_roi(roi) {
        // Initialize parameters with default (identity) values
        p_cosA.set(1.0f);
        p_sinA.set(0.0f);
        p_cx.set(0);
        p_cy.set(0);
        p_src_width.set(0);
        p_src_height.set(0);
        p_roi_x.set(0);
        p_roi_y.set(0);
    }

    std::string getName() const override {
        return "Rotate";
    }

    cv::Mat apply(const cv::Mat& srcImg) override;

    void setAngle(int angle) {
        m_angle_deg = angle;
    }
    int getAngle() const {
        return m_angle_deg;
    }

    void setROI(cv::Rect rect) {
        m_roi = rect;
    }
    cv::Rect getROI() const {
        return m_roi;
    }

    // --- Halide Implementations ---
    void prepareParameters(const cv::Mat& srcImg) override;

    Halide::Func buildGraph(Halide::Func srcImg, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;

    void getOutputDimensions([[maybe_unused]] int srcWidth, [[maybe_unused]] int srcHeight,
                             int& dstWidth, int& dstHeight) const override {
        if (m_roi.empty()) {
            // For 90° and 270° rotations, swap width and height
            int normalizedAngle = ((m_angle_deg % 360) + 360) % 360;
            if (normalizedAngle == 90 || normalizedAngle == 270) {
                dstWidth = srcHeight;
                dstHeight = srcWidth;
            } else {
                dstWidth = srcWidth;
                dstHeight = srcHeight;
            }
        } else {
            dstWidth = m_roi.width;
            dstHeight = m_roi.height;
        }
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
        // 1. Check if the Source Image is empty
        if (srcImg.empty()) {
            std::cerr << "Error: input image is empty\n";
            return cv::Mat();
        }

        // 2. Get the Image Dimension
        const int imgW = srcImg.cols;
        const int imgH = srcImg.rows;

        // 3. Normalize angle to [0, 360)
        int normalizedAngle = ((angle_deg % 360) + 360) % 360;

        // 4. Handle ROI or calculate output dimensions
        int dstW, dstH;
        if (!roi.empty()) {
            dstW = roi.width;
            dstH = roi.height;
        } else if (normalizedAngle == 90 || normalizedAngle == 270) {
            // For 90° and 270°, swap dimensions
            dstW = imgH;
            dstH = imgW;
        } else {
            dstW = imgW;
            dstH = imgH;
        }

        // 5. Create Destination Image
        cv::Mat dstImg(dstH, dstW, srcImg.type(), cv::Scalar(0));

        // 6. Get the Center Points
        const float srcCx = imgW / 2.0f;
        const float srcCy = imgH / 2.0f;
        const float dstCx = dstW / 2.0f;
        const float dstCy = dstH / 2.0f;

        // 7. Precalculation
        const double angle_rad = angle_deg * M_PI / 180.0;
        const double cosA = std::cos(angle_rad);
        const double sinA = std::sin(angle_rad);

        // 8. Get the pointers of first pixel each lines in an array for better access
        std::vector<const T*> srcRowPtrs(imgH);
        for (int y = 0; y < imgH; y++) {
            srcRowPtrs[y] = srcImg.ptr<T>(y);
        }

        // 9. Create a Lambda Function for Boundary-Check
        auto inBounds = [imgW, imgH](int x, int y) -> bool {
            return static_cast<unsigned>(x) < static_cast<unsigned>(imgW) &&
                   static_cast<unsigned>(y) < static_cast<unsigned>(imgH);
        };

        // clang-format off
        // 10. Iteration though the Destination pixels using OpenMP for Parallelism
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < dstH; y++) {
            // 10.1 Get the pointer of first pixel each line in the Destination Image
            T* outPtr = dstImg.ptr<T>(y);

            // 10.2 Execute pixel-wise using Inverse Mapping
            for (int x = 0; x < dstW; x++) {
                // Calculate offset from destination center
                float dx = x - dstCx;
                float dy = y - dstCy;

                // Inverse rotation to find source coordinates
                float oldX = dx * cosA + dy * sinA + srcCx;
                float oldY = -dx * sinA + dy * cosA + srcCy;

                int i = static_cast<int>(oldX);
                int j = static_cast<int>(oldY);
                float a = oldX - i;
                float b = oldY - j;

                // 10.2.2 Calculate the weights
                float wx[4], wy[4];
                for (int m = 0; m < 4; m++) {
                    wx[m] = ImageUtils::calculateCubicWeight(a - (m - 1));
                }
                for (int n = 0; n < 4; n++) {
                    wy[n] = ImageUtils::calculateCubicWeight(b - (n - 1));
                }

                // --- Path A. Gray Image ---
                if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>) {
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

                    if constexpr (std::is_same_v<T, uint8_t>) {
                        outPtr[x] = static_cast<uchar>(std::clamp(sum, 0.0f, 255.0f));
                    } else {
                        outPtr[x] = static_cast<uint16_t>(std::clamp(sum, 0.0f, 65535.0f));
                    }

                }

                // --- Path B. Color Image ---
                else if constexpr (std::is_same_v<T, cv::Vec3b> || std::is_same_v<T, cv::Vec3w>) {
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
        return dstImg;
    }
};

#endif  // ROTATE_H