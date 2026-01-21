#ifndef IMAGE_CONTROLLER_H
#define IMAGE_CONTROLLER_H

#include <opencv2/core.hpp>

#include "../core/image_pipeline.h"

/**
 * @brief The Data Model for the GUI.
 * This struct contains ALL parameters that the user can change via sliders or buttons.
 * It represents the "Single Source of Truth" for the image look. "Single Source of Truth" is a
 * design principle that states that there should be a single source of truth for the state of the
 * system. In this case, the ImageState struct is the single source of truth for the image look.
 * In CPU Mode, if a slider is used (the value is not zero), this operation is added to the
 * pipeline, if zero, it will be removed.
 */
struct ImageState {
    // --- Light ---
    float exposure = 0.0f;    // Range: -5.0 to +5.0
    float contrast = 0.0f;    // Range: -100 to +100
    float highlight = 0.0f;   // Range: -100 to +100
    float shadow = 0.0f;      // Range: -100 to +100
    float white = 0.0f;       // Range: -100 to +100
    float black = 0.0f;       // Range: -100 to +100
    float brightness = 0.0f;  // Range: -100 to +100

    // --- Color ---
    float saturation = 0.0f;   // Range: -100 to +100
    float vibrance = 0.0f;     // Range: -100 to +100
    float temp = 0.0f;         // Range: -100 to +100
    float tint = 0.0f;         // Range: -100 to +100
    float tintMagenta = 0.0f;  // Range: -100 to +100

    // --- Detail ---
    float sharpen = 0.0f;  // Range: -100 to +100
    float clarity = 0.0f;  // Range: -100 to +100
    float denoise = 0.0f;  // Range: -100 to +100

    // --- Effects ---
    std::vector<std::string> activeEffects;

    // --- Geometry ---
    float rotation = 0.0f;     // Degrees, Range: -180 to +180
    int flip = -1;             // -1 = None, 0 = Vertical, 1 = Horizontal
    int resizeWidth = 0;       // 0 = Keep Original
    int resizeHeight = 0;      // 0 = Keep Original (if both 0)
    float resizeRatio = 0.0f;  // 0.0 = Use Width/Height. >0 = Use Height * Ratio
    cv::Rect cropRect;         // Empty = No crop
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

    // Set the Image in the Pipeline
    void setImage(const cv::Mat& img);

    // Update the ImageState and rebuild the Pipeline
    void update(const ImageState& state);

    // Process the Image
    cv::Mat process();

    // Getters for specific UI needs
    ImagePipeline& getPipeline() {
        return m_pipeline;
    }

    // Set the Preview Mode
    void setPreviewMode(bool enabled);

    // Check if Preview Mode is enabled
    bool isPreviewMode() const {
        return m_isPreviewMode;
    }

   private:
    ImagePipeline m_pipeline;
    ImageState m_currentState;

    // Image Storage
    cv::Mat m_fullResImage;  // The original full resolution image
    cv::Mat m_previewImage;  // A downscaled version for fast preview
    bool m_isPreviewMode = true;

    // Stats Cache (to avoid recalculating on every process() call)
    float m_cachedMinL = 0.0f;
    float m_cachedMaxL = 1.0f;
    bool m_statsValid = false;

    // Helper to rebuild CPU pipeline from state
    void rebuildPipeline(const ImageState& state);
};

#endif  // IMAGE_CONTROLLER_H
