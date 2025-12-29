// neue mit Parallel

#ifndef COLOR_SPACE_H
#define COLOR_SPACE_H

#include <opencv2/opencv.hpp>

class ColorSpace {
   public:
    /**
     * @brief Convert image from BGR to HSL color space
     * @param bgrImg Input image in BGR color space
     * @return Image converted to HSL color space (32FC3)
     */
    static cv::Mat convertBGR2HSL(const cv::Mat& bgrImg);

    /**
     * @brief Convert image from HSL to BGR color space
     * @param hslImg Input image in HSL color space
     * @return Image converted to BGR color space (original depth)
     */
    static cv::Mat convertHSL2BGR(const cv::Mat& hslImg, int bitDepth);

   private:
    /**
     * @brief Convert image from BGR to HSL color space
     * @param bgrImg Input image in BGR color space
     * @return Image converted to HSL color space (32FC3)
     */
    template <typename T>
    static cv::Mat convertBGR2HSLTemplate(const cv::Mat& bgrImg) {
        // 1. Check if the input image is empty
        if (bgrImg.empty())
            return cv::Mat();

        // 2. Create output image
        cv::Mat hslImg(bgrImg.size(), CV_32FC3);

        // 3. Calculate scale inverse
        const float scaleInv =
            std::is_same<T, cv::Vec3b>::value ? (1.0f / 255.0f) : (1.0f / 65535.0f);
        const int rows = bgrImg.rows;
        const int cols = bgrImg.cols;

        // 4. Epsilon for division by zero
        const float eps = 1e-7f;

        // 5. Parallelize over rows
        // clang-format off
        #pragma omp parallel for
        // clang-format on

        for (int y = 0; y < rows; y++) {
            // 6. Get row pointers
            const auto* rowSrc = bgrImg.ptr<T>(y);
            using ValueType = typename T::value_type;
            const ValueType* srcPtr = reinterpret_cast<const ValueType*>(rowSrc);

            // 7. Get destination row pointer
            float* dstPtr = hslImg.ptr<float>(y);

            // Use restrict to tell the compiler that the pointers do not overlap
            const ValueType* __restrict s = srcPtr;
            float* __restrict d = dstPtr;

            // 8. Parallelize over columns
            // clang-format off
            #pragma omp simd
            // clang-format on
            for (int x = 0; x < cols; x++) {
                // 9. Get pixel values
                int idx = x * 3;

                // 10. Convert to float and scale
                float B = static_cast<float>(s[idx]) * scaleInv;
                float G = static_cast<float>(s[idx + 1]) * scaleInv;
                float R = static_cast<float>(s[idx + 2]) * scaleInv;

                // 11. Calculate maximum and minimum
                float cMax = std::max({R, G, B});
                float cMin = std::min({R, G, B});
                float delta = cMax - cMin;

                // 12. Calculate Lightness
                float L = (cMax + cMin) * 0.5f;

                // --- BRANCHLESS LOGIC START ---

                // 13. Calculate Saturation
                // Calculate divisor and saturation
                float divisor = 1.0f - std::abs(2.0f * L - 1.0f);
                float S = delta / (divisor + eps);

                // 14. Hue Calculation "Branchless"

                float offset;   // Offset for hue calculation
                float segment;  // Segment for hue calculation

                // Calculate offset and segment
                if (cMax == R) {
                    segment = G - B;
                    offset = 0.0f;
                } else if (cMax == G) {
                    segment = B - R;
                    offset = 2.0f;
                } else {  // cMax == B
                    segment = R - G;
                    offset = 4.0f;
                }

                // Calculate H
                float H = (segment / (delta + eps)) + offset;

                H *= 60.0f;

                // Wrap H to [0, 360)
                H += (H < 0.0f) * 360.0f;

                // Clean up for gray values
                bool isGray = (delta < 0.00001f);
                if (isGray) {
                    H = 0.0f;
                    S = 0.0f;
                }

                // --- BRANCHLESS LOGIC END ---

                // 15. Store values
                d[idx] = H;
                d[idx + 1] = S;
                d[idx + 2] = L;
            }
        }
        return hslImg;
    }

    /**
     * @brief Convert image from HSL to BGR color space
     * @param hslImg Input image in HSL color space
     * @param bitDepth The bit depth of the original image (8 or 16)
     * @return Image converted to BGR color space (original depth)
     */
    template <int T, typename vectorT>
    static cv::Mat convertHSL2BGRTemplate(const cv::Mat& hslImg, int bitDepth) {
        // 1. Check if the input image is empty
        if (hslImg.empty())
            return cv::Mat();

        // 2. Check if the input image is CV_32FC3
        if (hslImg.type() != CV_32FC3) {
            std::cerr << "Error: Input must be CV_32FC3\n";
            return cv::Mat();
        }

        // 3. Setup
        cv::Mat bgrImg(hslImg.size(), T);
        const float scale = (bitDepth == 8) ? 255.0f : 65535.0f;
        const int rows = hslImg.rows;
        const int cols = hslImg.cols;

        using ValueType = typename vectorT::value_type;

        // 4. Parallelisierung über Zeilen (Multithreading)
        // clang-format off
        #pragma omp parallel for
        // clang-format on
        for (int y = 0; y < rows; y++) {
            // 5. Get row pointers
            const float* srcPtr = hslImg.ptr<float>(y);

            // 6. Get destination row pointer
            ValueType* dstPtr = reinterpret_cast<ValueType*>(bgrImg.ptr<vectorT>(y));

            // Compiler Hints für Pointer Aliasing (hilft jedem Compiler)
            const float* __restrict s = srcPtr;
            ValueType* __restrict d = dstPtr;

            // 7. Vektorisierung der Pixel (SIMD)
            // clang-format off
            #pragma omp simd
            // clang-format on
            for (int x = 0; x < cols; x++) {
                int idx = x * 3;

                // 8. Get pixel values
                float H = s[idx];
                float S = s[idx + 1];
                float L = s[idx + 2];

                // 9. --- MATHEMATIK (Branchless) ---

                // Chroma: C = (1 - |2L - 1|) * S
                float absL = std::abs(2.0f * L - 1.0f);
                float C = (1.0f - absL) * S;

                // X Calculation: C * (1 - |(H/60) % 2 - 1|)
                // Manual fmod for better performance/vektorization
                float H_prime = H * (1.0f / 60.0f);
                float fmod_approx = H_prime - 2.0f * std::floor(H_prime * 0.5f);
                float X = C * (1.0f - std::abs(fmod_approx - 1.0f));

                // Match Value m
                float m = L - C * 0.5f;

                // 10. Sector Determination (Masking instead of if/else)
                float isSec0 = (float)(H_prime < 1.0f);
                float isSec1 = (float)(H_prime >= 1.0f && H_prime < 2.0f);
                float isSec2 = (float)(H_prime >= 2.0f && H_prime < 3.0f);
                float isSec3 = (float)(H_prime >= 3.0f && H_prime < 4.0f);
                float isSec4 = (float)(H_prime >= 4.0f && H_prime < 5.0f);
                float isSec5 = (float)(H_prime >= 5.0f);

                // 11. Branchless Assignment through Multiplication
                float R_temp = (isSec0 + isSec5) * C + (isSec1 + isSec4) * X;
                float G_temp = (isSec1 + isSec2) * C + (isSec0 + isSec3) * X;
                float B_temp = (isSec3 + isSec4) * C + (isSec2 + isSec5) * X;

                // 12. --- STORAGE ---

                d[idx] = cv::saturate_cast<ValueType>((B_temp + m) * scale + 0.5f);
                d[idx + 1] = cv::saturate_cast<ValueType>((G_temp + m) * scale + 0.5f);
                d[idx + 2] = cv::saturate_cast<ValueType>((R_temp + m) * scale + 0.5f);
            }
        }
        return bgrImg;
    }
};

#endif