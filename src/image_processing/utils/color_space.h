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
    template <typename T>
    static cv::Mat convertBGR2HSLTemplate(const cv::Mat& bgrImg) {
        // check if the input image is empty
        if (bgrImg.empty()) {
            std::cerr << "Error in convertBGR2HSLTemplate: empty input image\n";
            return cv::Mat();
        }
        if (bgrImg.type() != CV_8UC3 && bgrImg.type() != CV_16UC3) {
            std::cerr << "Error in convertBGR2HSLTemplate: supported only BGR Image\n";
            return cv::Mat();
        }

        cv::Mat hslImg(bgrImg.size(), CV_32FC3);

        // Scale depending on bit depth 8-->255, 16-->65535
        float scale = std::is_same<T, cv::Vec3b>::value ? 255.0f : 65535.0f;

        for (int y = 0; y < bgrImg.rows; y++) {
            const T* bgrPtr = bgrImg.ptr<T>(y);
            cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);

            for (int x = 0; x < bgrImg.cols; x++) {
                // Step 1Normalization
                float B = bgrPtr[x][0] / scale;
                float G = bgrPtr[x][1] / scale;
                float R = bgrPtr[x][2] / scale;

                // Step 2: cMax, cMin, delta
                float cMax = std::max({R, G, B});
                float cMin = std::min({R, G, B});
                float delta = cMax - cMin;

                // Step 3: Lightness
                float L = (cMax + cMin) / 2.0f;

                // Step 4: Saturation S
                float S = 0.0f;
                if (delta == 0) {
                    S = 0.0f;
                } else {
                    S = delta / (1 - std::fabs(2 * L - 1));
                }

                // Step 5: Hue H
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

                hslPtr[x] = cv::Vec3f(H, S, L);
            }
        }
        return hslImg;
    }

    /**
     * @brief Convert image from BGR to HSL color space
     * @param bgrImg Input image in BGR color space
     * @return Image converted to HSL color space (32FC3)
     */
    static cv::Mat convertBGR2HSL(const cv::Mat& bgrImg) {
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
     * @param bitDepth The bit depth of the original image (8 or 16)
     * @return Image converted to BGR color space (original depth)
     */
    template <int T, typename vectorT>
    static cv::Mat convertHSL2BGRTemplate(const cv::Mat& hslImg, int bitDepth) {
        // check if the input image is empty
        if (hslImg.empty()) {
            std::cerr << "Error in convertHSLBGRTemplate: empty input image\n";
            return cv::Mat();
        }
        if (hslImg.type() != CV_32FC3) {
            std::cerr << "Error in convertHSL2BGRTemplate: supported only float image\n";
            return cv::Mat();
        }

        cv::Mat bgrImg(hslImg.size(), T);

        // scale depending on the bit depth
        float scale = (bitDepth == 8) ? 255.0f : 65535.0f;

        for (int y = 0; y < hslImg.rows; y++) {
            const cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);
            vectorT* bgrPtr = bgrImg.ptr<vectorT>(y);

            for (int x = 0; x < hslImg.cols; x++) {
                float H = hslPtr[x][0];
                float S = hslPtr[x][1];
                float L = hslPtr[x][2];

                float C = (1 - std::fabs(2 * L - 1)) * S;  // Chroma

                float X = C * (1 - std::fabs(std::fmod((H / 60.0f), 2.0f) - 1.0f));

                float m = L - (C / 2);  // shift

                // RBG-Basis
                float R1 = 0.0f, G1 = 0.0f, B1 = 0.0f;
                if (H < 60) {
                    R1 = C;
                    G1 = X;
                    B1 = 0;
                } else if (H < 120) {
                    R1 = X;
                    G1 = C;
                    B1 = 0;
                } else if (H < 180) {
                    R1 = 0;
                    G1 = C;
                    B1 = X;
                } else if (H < 240) {
                    R1 = 0;
                    G1 = X;
                    B1 = C;
                } else if (H < 300) {
                    R1 = X;
                    G1 = 0;
                    B1 = C;
                } else {
                    R1 = C;
                    G1 = 0;
                    B1 = X;
                }

                bgrPtr[x] = vectorT(
                    static_cast<typename vectorT::value_type>(std::round((B1 + m) * scale)),
                    static_cast<typename vectorT::value_type>(std::round((G1 + m) * scale)),
                    static_cast<typename vectorT::value_type>(std::round((R1 + m) * scale)));
            }
        }
        return bgrImg;
    }

    /**
     * @brief Convert image from HSL to BGR color space
     * @param hslImg Input image in HSL color space
     * @return Image converted to BGR color space (original depth)
     */
    static cv::Mat convertHSL2BGR(const cv::Mat& hslImg, int bitDepth) {
        // check if the input image is empty
        if (hslImg.empty()) {
            std::cerr << "Error in convertHSL2BGR: the input image is empty\n";
            return cv::Mat();
        }
        if (hslImg.type() != CV_32FC3) {
            std::cerr << "Error in convertHSL2BGR: supported only float image\n";
            return cv::Mat();
        }
        if (bitDepth == 8) {
            return convertHSL2BGRTemplate<CV_8UC3, cv::Vec3b>(hslImg, bitDepth);
        } else if (bitDepth == 16) {
            return convertHSL2BGRTemplate<CV_16UC3, cv::Vec3w>(hslImg, bitDepth);
        } else {
            std::cerr << "Error in convertHSL2BGR: unsupported bit depth (only 8 oder 16)\n";
            return cv::Mat();
        }
    }
};

#endif  // COLOR_SPACE.H