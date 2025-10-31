#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include <opencv2/core/hal/interface.h>

#include <cmath>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>
#include <vector>

/**
 * @class ImageProcessor
 * @brief Professional-grade image processing engine for photo manufacturing applications
 *
 * Provides comprehensive image manipulation capabilities including geometric transformations,
 * color space conversions, histogram analysis, and advanced color grading operations.
 * All operations are optimized for performance and quality.
 */
class ImageProcessor {
   public:
    ImageProcessor() = default;
    ~ImageProcessor() = default;

    // =========================================================================
    // GEOMETRIC TRANSFORMATIONS
    // =========================================================================

    /**
     * @brief Crop image to specified rectangular region
     * @param iPut Input source image no matter which format
     * @param roi Region of interest to extract (x, y, width, height)
     * @return Cropped image region
     * @throws std::invalid_argument if ROI exceeds image boundaries
     */
    cv::Mat cropImg(const cv::Mat& iPut, const cv::Rect& roi);

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

    /**
     * @brief Rotate image by specified angle around center
     * @brief Interpolation method: bicubic
     * @param iPut Input source image
     * @param angle Rotation angle in degrees (positive = counter-clockwise)
     * @return Rotated image with preserved content
     */
    static cv::Mat rotateImg(const cv::Mat& iPut, int angle_deg, cv::Rect roi) {
        switch (iPut.type()) {
            case CV_8UC1:
                return rotateImgTemplate<uchar>(iPut, angle_deg, roi);
            case CV_8UC3:
                return rotateImgTemplate<cv::Vec3b>(iPut, angle_deg, roi);
            case CV_16UC3:
                return rotateImgTemplate<cv::Vec3w>(iPut, angle_deg, roi);
            default:
                std::cerr << "rotateImg: unsupported type\n";
                return cv::Mat();
        }
    }

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

    /**
     * @brief Mirror image along horizontal or vertical axis
     * @param iPut Input source image
     * @param horizontal True for horizontal mirroring, false for vertical
     * @return Mirrored image
     */
    static cv::Mat flipImg(const cv::Mat& iPut, int flipCode) {
        if (iPut.type() == CV_16UC3) {
            return flipImgTemplate<cv::Vec3w>(iPut, flipCode);
        } else if (iPut.type() == CV_8UC3) {
            return flipImgTemplate<cv::Vec3b>(iPut, flipCode);
        } else if (iPut.type() == CV_8UC1) {
            return flipImgTemplate<uchar>(iPut, flipCode);
        } else {
            std::cerr << "Error in flipImg: unsupported datatype\n";
            return cv::Mat();
        }
    }

    // =========================================================================
    // COLOR SPACE CONVERSIONS
    // =========================================================================

    /**
     * @brief Convert image from BGR to HSL color space
     * @param bgrImg Input image in BGR color space
     * @return Image converted to HSL color space (32FC3)
     */
    template <typename T>
    cv::Mat convertBGR2HSLTemplate(const cv::Mat& iPut) {
        // check if the input image is empty
        if (iPut.empty()) {
            std::cerr << "Error in convertBGR2HSLTemplate: the input image is empty\n";
            return cv::Mat();
        }
        cv::Mat oPut(iPut.size(), CV_32FC3);

        for (int y = 0; y < iPut.rows; y++) {
            const T* iPtr = iPut.ptr<T>(y);
            cv::Vec3f* oPtr = oPut.ptr<cv::Vec3f>(y);

            for (int x = 0; x < iPut.cols; x++) {
                // Schritt 1: Normalisierung
                float B = iPtr[x][0] / 255.0f;
                float G = iPtr[x][1] / 255.0f;
                float R = iPtr[x][2] / 255.0f;

                // Schritt 2: cMax, cMin, delta
                float cMax = std::max({R, G, B});
                float cMin = std::min({R, G, B});
                float delta = cMax - cMin;

                // Schritt 3: Lightness
                float L = (cMax + cMin) / 2.0f;

                // Schritt 4: Saturation S
                float S = 0.0f;
                if (delta == 0) {
                    S = 0.0f;
                } else {
                    S = delta / (1 - std::fabs(2 * L - 1));
                }

                // Schritt 5: Hue H
                float H = 0.0f;
                if (delta == 0) {
                    H = 0;
                } else if (cMax == R) {
                    H = std::fmod((G - B) / delta, 6.0f);
                } else if (cMax == G) {
                    H = (B - R) / delta + 2;
                } else if (cMax == B) {
                    H = (R - G) / delta + 4;
                }
                H *= 60.0f;
                if (H < 0) {
                    H += 360.0f;
                }

                oPtr[x] = cv::Vec3f(H, S, L);
            }
        }
        return oPut;
    }

    /**
     * @brief Convert image from BGR to HSL color space
     * @param bgrImg Input image in BGR color space
     * @return Image converted to HSL color space (32FC3)
     */
    cv::Mat convertBGR2HSL(const cv::Mat& bgrImg) {
        if (bgrImg.type() == CV_8UC3) {
            return convertBGR2HSLTemplate<cv::Vec3b>(bgrImg);
        } else if (bgrImg.type() == CV_16UC3) {
            return convertBGR2HSLTemplate<cv::Vec3w>(bgrImg);
        } else {
            std::cerr << "Error in converBGR2HSL: unsupported input data type\n";
            return cv::Mat();
        }
    }

    /**
     * @brief Convert image from HSL to BGR color space
     * @param hslImg Input image in HSL color space
     * @return Image converted to BGR color space (original depth)
     */
    cv::Mat convertHSL2BGR(const cv::Mat& hslImg);

    // =========================================================================
    // HISTOGRAM ANALYSIS
    // =========================================================================

    /**
     * @brief Calculate image histogram for tonal distribution analysis
     * @param iPut Input image (single or multi-channel)
     * @param bins Number of histogram bins (default: 256)
     * @return Vector of histogram bin counts
     */
    cv::Mat Histogram(const cv::Mat& iPut);

    /**
     * @brief Calculate normalized cumulative distribution function
     * @param histogram Input histogram from Histogram() function
     * @return Normalized CDF values ranging from 0.0 to 1.0
     */
    std::vector<float> normedCDF(const std::vector<int>& histogram);

    // =========================================================================
    // LUMINANCE AND TONAL ADJUSTMENTS
    // =========================================================================

    /**
     * @brief Adjust overall image brightness
     * @param iPut Input source image
     * @param brightness Adjustment value (-100 to +100)
     * @return Brightness-adjusted image
     */
    cv::Mat adjustBrightness(const cv::Mat& iPut, int brightness);

    /**
     * @brief Adjust image contrast
     * @param iPut Input source image
     * @param contrast Adjustment value (-100 to +100)
     * @return Contrast-adjusted image
     */
    cv::Mat adjustContrast(const cv::Mat& iPut, int contrast);

    /**
     * @brief Adjust highlight regions intensity
     * @param iPut Input source image
     * @param highlight Adjustment value (-100 to +100)
     * @return Highlight-adjusted image
     */
    cv::Mat adjustHightlight(const cv::Mat& iPut, int highlight);

    /**
     * @brief Adjust shadow regions intensity
     * @param iPut Input source image
     * @param shadow Adjustment value (-100 to +100)
     * @return Shadow-adjusted image
     */
    cv::Mat adjustShadow(const cv::Mat& iPut, int shadow);

    /**
     * @brief Adjust white point level
     * @param iPut Input source image
     * @param white Adjustment value (0 to 100)
     * @return White point adjusted image
     */
    cv::Mat adjustWhite(const cv::Mat& iPut, int white);

    /**
     * @brief Adjust black point level
     * @param iPut Input source image
     * @param black Adjustment value (0 to 100)
     * @return Black point adjusted image
     */
    cv::Mat adjustBlack(const cv::Mat& iPut, int black);

    /**
     * @brief Automatic lighting correction based on histogram analysis
     * @param iPut Input source image
     * @param strength Correction strength (0 to 100)
     * @return Auto-corrected image
     */
    cv::Mat autoCorrectLight(const cv::Mat& iPut, int strength);

    // =========================================================================
    // COLOR AND TEMPERATURE ADJUSTMENTS
    // =========================================================================

    /**
     * @brief Adjust color temperature (warm/cool balance)
     * @param iPut Input source image
     * @param temperature Adjustment value (-100 to +100)
     * @return Temperature-adjusted image
     */
    cv::Mat adjustTemperature(const cv::Mat& iPut, int temperature);

    /**
     * @brief Adjust magenta/green tint balance
     * @param iPut Input source image
     * @param tintMagenta Adjustment value (-100 to +100)
     * @return Tint-adjusted image
     */
    cv::Mat adjustTintMagenta(const cv::Mat& iPut, int tintMagenta);

    /**
     * @brief Adjust color vibrance (non-linear saturation)
     * @param iPut Input source image
     * @param vibrance Adjustment value (-100 to +100)
     * @return Vibrance-adjusted image
     */
    cv::Mat adjustVibrance(const cv::Mat& iPut, int vibrance);

    /**
     * @brief Adjust color saturation
     * @param iPut Input source image
     * @param saturation Adjustment value (-100 to +100)
     * @return Saturation-adjusted image
     */
    cv::Mat adjustSaturation(const cv::Mat& iPut, int saturation);

    // =========================================================================
    // DETAIL AND SHARPNESS ENHANCEMENTS
    // =========================================================================

    /**
     * @brief Adjust image clarity (local contrast enhancement)
     * @param iPut Input source image
     * @param clarity Adjustment value (-100 to +100)
     * @return Clarity-enhanced image
     */
    cv::Mat adjustClarity(const cv::Mat& iPut, int clarity);

    /**
     * @brief Apply sharpening filter to enhance image details
     * @param iPut Input source image
     * @param sharpening Adjustment value (0 to 100)
     * @return Sharpened image
     */
    cv::Mat adjustSharpening(const cv::Mat& iPut, int sharpening);

   private:
    // Performance optimization cache
    mutable cv::Mat m_cachedHSL;
    mutable bool m_hasCachedHSL = false;
};

#endif  // IMAGE_PROCESSING_H