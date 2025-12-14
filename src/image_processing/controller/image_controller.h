#ifndef IMAGE_CONTROLLER_H
#define IMAGE_CONTROLLER_H

#include <opencv2/core.hpp>

#include "../core/image_pipeline.h"

/**
 * @brief The Data Model for the GUI.
 * This struct contains ALL parameters that the user can change via sliders or buttons.
 * It represents the "Single Source of Truth" for the image look.
 */
struct ImageState {
    // --- Light ---
    float exposure = 0.0f;    // Range: -5.0 to +5.0 (approx)
    float contrast = 1.0f;    // Range: 0.0 to 2.0 (1.0 = neutral)
    float highlight = 0.0f;   // Range: -100 to +100
    float shadow = 0.0f;      // Range: -100 to +100
    float white = 0.0f;       // Range: -100 to +100
    float black = 0.0f;       // Range: -100 to +100
    float brightness = 0.0f;  // Range: -100 to +100

    // --- Color ---
    float saturation = 1.0f;   // Range: 0.0 to 2.0 (1.0 = neutral)
    float vibrance = 0.0f;     // Range: -100 to +100
    float temp = 0.0f;         // Range: -100 to +100 (Kelvin shift)
    float tint = 0.0f;         // Range: -100 to +100
    float tintMagenta = 0.0f;  // Range: -100 to +100 (Specific magenta tint)

    // --- Detail ---
    float sharpen = 0.0f;  // Range: 0.0 to 100.0
    float clarity = 0.0f;  // Range: 0.0 to 100.0
    float denoise = 0.0f;  // Range: 0.0 to 100.0 (Blend factor)

    // --- Geometry ---
    float rotation = 0.0f;     // Degrees
    int flip = -1;             // -1 = None, 0 = Vertical, 1 = Horizontal
    int resizeWidth = 0;       // 0 = Keep Original
    int resizeHeight = 0;      // 0 = Keep Original (if both 0)
    float resizeRatio = 0.0f;  // 0.0 = Use Width/Height. >0 = Use Height * Ratio
    cv::Rect cropRect;         // Empty = No crop

    // --- System / Meta ---
    bool enableBeforeAfter = false;  // Example for UI features
};

/**
 * @brief The Controller that translates ImageState into Engine commands.
 *
 * GUI Developer usage:
 * 1. modify the `ImageState` struct when sliders move.
 * 2. call `controller.update(state)` to refresh the engine.
 * 3. call `controller.process()` to get the image.
 */
class ImageController {
   public:
    ImageController();
    ~ImageController();

    // Setup
    void loadImage(const cv::Mat& result);
    void setImage(const cv::Mat& img);

    // Main Interaction
    void update(const ImageState& state);

    // Execution
    cv::Mat process();

    // Getters for specific UI needs (e.g. Histogram)
    ImagePipeline& getPipeline() {
        return m_pipeline;
    }

   private:
    ImagePipeline m_pipeline;
    ImageState m_currentState;

    // Stats Cache (to avoid recalculating on every process() call)
    float m_cachedMinL = 0.0f;
    float m_cachedMaxL = 1.0f;
    // float m_lastExposure = 0.0f;
    bool m_statsValid = false;

    // Helper to rebuild JIT pipeline from state
    void rebuildPipeline(const ImageState& state);
};

#endif  // IMAGE_CONTROLLER_H
