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
        auto start = std::chrono::high_resolution_clock::now();
        // check if the input image is empty
        if (bgrImg.empty())
            return cv::Mat();

        cv::Mat hslImg(bgrImg.size(), CV_32FC3);

        const float scaleInv =
            std::is_same<T, cv::Vec3b>::value ? (1.0f / 255.0f) : (1.0f / 65535.0f);
        const int rows = bgrImg.rows;
        const int cols = bgrImg.cols;

        // Kleines Epsilon, um Division durch 0 zu verhindern, ohne zu branchen
        const float eps = 1e-7f;

#pragma omp parallel for
        for (int y = 0; y < rows; y++) {
            const auto* rowSrc = bgrImg.ptr<T>(y);
            using ValueType = typename T::value_type;
            const ValueType* srcPtr = reinterpret_cast<const ValueType*>(rowSrc);
            float* dstPtr = hslImg.ptr<float>(y);

            // Compiler-Hint: Pointer überlappen sich nicht (hilft Vektorisierung)
            const ValueType* __restrict s = srcPtr;
            float* __restrict d = dstPtr;

#pragma omp simd
            for (int x = 0; x < cols; x++) {
                int idx = x * 3;

                float B = static_cast<float>(s[idx]) * scaleInv;
                float G = static_cast<float>(s[idx + 1]) * scaleInv;
                float R = static_cast<float>(s[idx + 2]) * scaleInv;

                float cMax = std::max({R, G, B});
                float cMin = std::min({R, G, B});
                float delta = cMax - cMin;

                // Lightness
                float L = (cMax + cMin) * 0.5f;

                // --- BRANCHLESS LOGIC START ---

                // Saturation
                // Divisor berechnen: 1 - |2L - 1|
                // Wir addieren 'eps', damit wir nie durch 0 teilen.
                float divisor = 1.0f - std::abs(2.0f * L - 1.0f);
                float S = delta / (divisor + eps);

                // Hue Calculation "Branchless"
                // Wir nutzen Ternary Operators ( ? : ). Clang auf M1 kann diese
                // oft in "vsel" (Vector Select) Instruktionen umwandeln.

                // Schritt 1: Welche Farbe ist Max? (Ergibt 1.0f oder 0.0f)
                // Wenn cMax == R, nutzen wir (G-B), sonst wenn G, (B-R), sonst (R-G)
                // Da Ternary hier komplex wird, nutzen wir einen Offset-Trick.

                float offset;
                float segment;

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

                // H berechnen
                // (segment / (delta + eps)) verhindert Crash bei Grau
                float H = (segment / (delta + eps)) + offset;

                H *= 60.0f;

                // Fmod/Wrap Ersatz ohne if:
                // Wenn H < 0, addiere 360.
                // (H < 0) ergibt 1 oder 0.
                H += (H < 0.0f) * 360.0f;

                // Clean up für Graustufen:
                // Wenn delta sehr klein ist, setzen wir S und H hart auf 0.
                // Das passiert am Ende via Maskierung.
                bool isGray = (delta < 0.00001f);
                if (isGray) {
                    H = 0.0f;
                    S = 0.0f;
                }

                // --- BRANCHLESS LOGIC END ---

                d[idx] = H;
                d[idx + 1] = S;
                d[idx + 2] = L;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "convertBGR2HSLTemplate time: " << duration.count() << " microseconds"
                  << std::endl;
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
        auto start = std::chrono::high_resolution_clock::now();
        if (hslImg.empty())
            return cv::Mat();
        if (hslImg.type() != CV_32FC3) {
            std::cerr << "Error: Input must be CV_32FC3\n";
            return cv::Mat();
        }

        // 2. Setup
        cv::Mat bgrImg(hslImg.size(), T);
        const float scale = (bitDepth == 8) ? 255.0f : 65535.0f;
        const int rows = hslImg.rows;
        const int cols = hslImg.cols;

        using ValueType = typename vectorT::value_type;

// 3. Parallelisierung über Zeilen (Multithreading)
// Funktioniert auf Mac, Linux, Windows gleich.
#pragma omp parallel for
        for (int y = 0; y < rows; y++) {
            const float* srcPtr = hslImg.ptr<float>(y);

            // reinterpret_cast auf den Ziel-Vektor-Typ
            ValueType* dstPtr = reinterpret_cast<ValueType*>(bgrImg.ptr<vectorT>(y));

            // Compiler Hints für Pointer Aliasing (hilft jedem Compiler)
            const float* __restrict s = srcPtr;
            ValueType* __restrict d = dstPtr;

// 4. Vektorisierung der Pixel (SIMD)
// "omp simd" ist der Standard-Weg, Vektorisierung zu erzwingen.
// Windows (MSVC) ignoriert dies oft, nutzt aber dank des
// branchless Codes unten seine eigene Auto-Vektorisierung (/O2).
#pragma omp simd
            for (int x = 0; x < cols; x++) {
                int idx = x * 3;

                // Daten Laden
                float H = s[idx];
                float S = s[idx + 1];
                float L = s[idx + 2];

                // --- MATHEMATIK (Branchless) ---

                // Chroma: C = (1 - |2L - 1|) * S
                float absL = std::abs(2.0f * L - 1.0f);
                float C = (1.0f - absL) * S;

                // X Calculation: C * (1 - |(H/60) % 2 - 1|)
                // Manuelles fmod für bessere Performance/Vektorisierung
                float H_prime = H * (1.0f / 60.0f);
                float fmod_approx = H_prime - 2.0f * std::floor(H_prime * 0.5f);
                float X = C * (1.0f - std::abs(fmod_approx - 1.0f));

                // Match Value m
                float m = L - C * 0.5f;

                // Sektoren-Bestimmung (Masking statt if/else)
                // Vergleichsoperatoren liefern 1.0f (wahr) oder 0.0f (falsch) nach Cast
                float isSec0 = (float)(H_prime < 1.0f);
                float isSec1 = (float)(H_prime >= 1.0f && H_prime < 2.0f);
                float isSec2 = (float)(H_prime >= 2.0f && H_prime < 3.0f);
                float isSec3 = (float)(H_prime >= 3.0f && H_prime < 4.0f);
                float isSec4 = (float)(H_prime >= 4.0f && H_prime < 5.0f);
                float isSec5 = (float)(H_prime >= 5.0f);

                // Branchless Zuweisung durch Multiplikation
                // R ist C in Sektor 0 und 5, X in Sektor 1 und 4
                float R_temp = (isSec0 + isSec5) * C + (isSec1 + isSec4) * X;
                float G_temp = (isSec1 + isSec2) * C + (isSec0 + isSec3) * X;
                float B_temp = (isSec3 + isSec4) * C + (isSec2 + isSec5) * X;

                // --- SPEICHERN ---

                // Runden & Skalieren & Clampen (Saturate)
                // (val + 0.5f) ist schneller "Round to Nearest" Trick
                d[idx] = cv::saturate_cast<ValueType>((B_temp + m) * scale + 0.5f);
                d[idx + 1] = cv::saturate_cast<ValueType>((G_temp + m) * scale + 0.5f);
                d[idx + 2] = cv::saturate_cast<ValueType>((R_temp + m) * scale + 0.5f);
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "convertHSL2BGRTemplate time: " << duration.count() << " microseconds"
                  << std::endl;
        return bgrImg;
    }
};
#endif

// old

// #ifndef COLOR_SPACE_H
// #define COLOR_SPACE_H
// #include <opencv2/opencv.hpp>

// class ColorSpace {
//    public:
//     /**
//      * @brief Convert image from BGR to HSL color space
//      * @param bgrImg Input image in BGR color space
//      * @return Image converted to HSL color space (32FC3)
//      */
//     static cv::Mat convertBGR2HSL(const cv::Mat& bgrImg);

//     /**
//      * @brief Convert image from HSL to BGR color space
//      * @param hslImg Input image in HSL color space
//      * @return Image converted to BGR color space (original depth)
//      */
//     static cv::Mat convertHSL2BGR(const cv::Mat& hslImg, int bitDepth);

//    private:
//     /**
//      * @brief Convert image from BGR to HSL color space
//      * @param bgrImg Input image in BGR color space
//      * @return Image converted to HSL color space (32FC3)
//      */
//     template <typename T>
//     static cv::Mat convertBGR2HSLTemplate(const cv::Mat& bgrImg) {
//         // check if the input image is empty
//         if (bgrImg.empty()) {
//             std::cerr << "Error in convertBGR2HSLTemplate: empty input image\n";
//             return cv::Mat();
//         }
//         if (bgrImg.type() != CV_8UC3 && bgrImg.type() != CV_16UC3) {
//             std::cerr << "Error in convertBGR2HSLTemplate: supported only BGR Image\n";
//             return cv::Mat();
//         }

//         cv::Mat hslImg(bgrImg.size(), CV_32FC3);

//         // Scale depending on bit depth 8-->255, 16-->65535
//         float scale = std::is_same<T, cv::Vec3b>::value ? 255.0f : 65535.0f;

//         for (int y = 0; y < bgrImg.rows; y++) {
//             const T* bgrPtr = bgrImg.ptr<T>(y);
//             cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);

//             for (int x = 0; x < bgrImg.cols; x++) {
//                 // Step 1Normalization
//                 float B = bgrPtr[x][0] / scale;
//                 float G = bgrPtr[x][1] / scale;
//                 float R = bgrPtr[x][2] / scale;

//                 // Step 2: cMax, cMin, delta
//                 float cMax = std::max({R, G, B});
//                 float cMin = std::min({R, G, B});
//                 float delta = cMax - cMin;

//                 // Step 3: Lightness
//                 float L = (cMax + cMin) / 2.0f;

//                 // Step 4: Saturation S
//                 float S = 0.0f;
//                 if (delta == 0) {
//                     S = 0.0f;
//                 } else {
//                     S = delta / (1 - std::fabs(2 * L - 1));
//                 }

//                 // Step 5: Hue H
//                 float H = 0.0f;
//                 if (delta == 0) {
//                     H = 0;
//                 } else if (cMax == R) {
//                     H = std::fmod((G - B) / delta, 6.0f);
//                 } else if (cMax == G) {
//                     H = (B - R) / delta + 2;
//                 } else if (cMax == B) {
//                     H = (R - G) / delta + 4;
//                 }
//                 H *= 60.0f;
//                 if (H < 0) {
//                     H += 360.0f;
//                 }

//                 hslPtr[x] = cv::Vec3f(H, S, L);
//             }
//         }
//         return hslImg;
//     }

//     /**
//      * @brief Convert image from HSL to BGR color space
//      * @param hslImg Input image in HSL color space
//      * @param bitDepth The bit depth of the original image (8 or 16)
//      * @return Image converted to BGR color space (original depth)
//      */
//     template <int T, typename vectorT>
//     static cv::Mat convertHSL2BGRTemplate(const cv::Mat& hslImg, int bitDepth) {
//         // check if the input image is empty
//         if (hslImg.empty()) {
//             std::cerr << "Error in convertHSLBGRTemplate: empty input image\n";
//             return cv::Mat();
//         }
//         if (hslImg.type() != CV_32FC3) {
//             std::cerr << "Error in convertHSL2BGRTemplate: supported only float image\n";
//             return cv::Mat();
//         }

//         cv::Mat bgrImg(hslImg.size(), T);

//         // scale depending on the bit depth
//         float scale = (bitDepth == 8) ? 255.0f : 65535.0f;

//         for (int y = 0; y < hslImg.rows; y++) {
//             const cv::Vec3f* hslPtr = hslImg.ptr<cv::Vec3f>(y);
//             vectorT* bgrPtr = bgrImg.ptr<vectorT>(y);

//             for (int x = 0; x < hslImg.cols; x++) {
//                 float H = hslPtr[x][0];
//                 float S = hslPtr[x][1];
//                 float L = hslPtr[x][2];

//                 float C = (1 - std::fabs(2 * L - 1)) * S;  // Chroma

//                 float X = C * (1 - std::fabs(std::fmod((H / 60.0f), 2.0f) - 1.0f));

//                 float m = L - (C / 2);  // shift

//                 // RBG-Basis
//                 float R1 = 0.0f, G1 = 0.0f, B1 = 0.0f;
//                 if (H < 60) {
//                     R1 = C;
//                     G1 = X;
//                     B1 = 0;
//                 } else if (H < 120) {
//                     R1 = X;
//                     G1 = C;
//                     B1 = 0;
//                 } else if (H < 180) {
//                     R1 = 0;
//                     G1 = C;
//                     B1 = X;
//                 } else if (H < 240) {
//                     R1 = 0;
//                     G1 = X;
//                     B1 = C;
//                 } else if (H < 300) {
//                     R1 = X;
//                     G1 = 0;
//                     B1 = C;
//                 } else {
//                     R1 = C;
//                     G1 = 0;
//                     B1 = X;
//                 }

//                 bgrPtr[x] = vectorT(
//                     static_cast<typename vectorT::value_type>(std::round((B1 + m) * scale)),
//                     static_cast<typename vectorT::value_type>(std::round((G1 + m) * scale)),
//                     static_cast<typename vectorT::value_type>(std::round((R1 + m) * scale)));
//             }
//         }
//         return bgrImg;
//     }
// };
// #endif  // COLOR_SPACE.H