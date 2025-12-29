#ifndef AUTO_LIGHT_H
#define AUTO_LIGHT_H

#include <opencv2/core.hpp>

/**
 * @brief Return Struct for the calculated optimal settings.
 * Maps directly to the ImageState.
 */
struct AutoLightSettings {
    float exposure = 0.0f;    // Range: -5.0 to +5.0
    float contrast = 0.0f;    // Range: -100 to +100
    float highlight = 0.0f;   // Range: -100 to +100
    float shadow = 0.0f;      // Range: -100 to +100
    float white = 0.0f;       // Range: -100 to +100
    float black = 0.0f;       // Range: -100 to +100
    float brightness = 0.0f;  // Range: -100 to +100
};

/**
 * @brief Analyzer Class for Auto-Light.
 * Only calculates parameters.
 * Stateless class -> can be used for both CPU and AOT Pipeline
 */
class AutoLight {
   public:
    /**
     * @brief Analyzes the image and returns optimal settings.
     * @param src Input image (BGR)
     * @return AutoLightSettings with suggested values.
     */
    static AutoLightSettings analyze(const cv::Mat& src);
};

#endif  // AUTO_LIGHT_H
