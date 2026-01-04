#include "image_controller.h"

#include <cmath>

// Light
#include "../operations/detail/clarity.h"
#include "../operations/detail/sharpen.h"
#include "../operations/light/black_adjust.h"
#include "../operations/light/brightness_adjust.h"
#include "../operations/light/contrast_adjust.h"
#include "../operations/light/exposure_adjust.h"
#include "../operations/light/highlight_adjust.h"
#include "../operations/light/shadow_adjust.h"
#include "../operations/light/white_adjust.h"

// Color
#include "../operations/color/saturation_adjust.h"
#include "../operations/color/tint_magenta.h"
#include "../operations/color/vibrance_adjust.h"
#include "../operations/color/white_balance.h"

// Effects
#include "../operations/effects/gray_image.h"
#include "../operations/effects/vintage1.h"

// Geometry
#include "../operations/denoise/denoise.h"
#include "../operations/geometry/crop.h"
#include "../operations/geometry/flip.h"
#include "../operations/geometry/rotate.h"
#include "../utils/image_resize.h"

ImageController::ImageController() {
    // Default: Use GPU mode
    m_pipeline.setFusionMode(true);
}

ImageController::~ImageController() {}

void ImageController::setImage(const cv::Mat& img) {
    m_pipeline.setImg(img);
    m_statsValid = false;  // Invalidate cache on new image
}

void ImageController::update(const ImageState& state) {
    // 1. Save state
    m_currentState = state;

    // 2.For CPU Strategy: rebuild the pipeline based on the fixed order, no matter when a operation
    // is enabled or disabled.
    rebuildPipeline(state);
}

// AOT Includes
#ifdef AOT_ENABLED  // AOT_ENABLED is defined in CMakeLists.txt
#include "../core/operation_registry.h"
#include "../utils/color_space.h"
#include "../utils/image_utils.h"
#include "HalideBuffer.h"
#include "photo_adjustment.h"
#endif

cv::Mat ImageController::process() {
#ifdef AOT_ENABLED

    // 1. Check if GPU Fusion is enabled. If not, fallback to CPU
    if (!m_pipeline.isFusionMode()) {
        return m_pipeline.process();
    }

    // 2. Check if image is empty
    if (m_pipeline.getImg().empty())
        return cv::Mat();

    // 3. Get image
    const cv::Mat& src = m_pipeline.getImg();

    // 4. Ensure 16-bit Input for AOT (High Precision)
    cv::Mat processingSrc;
    if (src.depth() == CV_8U) {
        // Upscale 8-bit to 16-bit
        src.convertTo(processingSrc, CV_16U, 256.0);  // 0-255 -> 0-65280
    } else if (src.depth() == CV_16U) {
        processingSrc = src;
    } else {
        // This fallback should not happen in this pipeline context. It's just for safety.
        src.convertTo(processingSrc, CV_16U);
    }

    // 5. Ensure 3 Channels for AOT Pipeline.
    // Gray Images will be converted to BGR because it's more flexible to edit an image in BGR space
    // than in grayscale space with just one channel.
    if (processingSrc.channels() == 1) {
        cv::cvtColor(processingSrc, processingSrc, cv::COLOR_GRAY2BGR);
    } else if (processingSrc.channels() == 4) {
        cv::cvtColor(processingSrc, processingSrc, cv::COLOR_BGRA2BGR);
    }

    // 6. Calculate Stats (Min/Max Luminance) for Determining The lower and upper Point of a
    // Luminance Area later Cache stats to avoid redundant computation because Min/Max Luminance
    // are found only once on the original Image to make Loop Fusion possible. On CPU Mode, the
    // Min/Max Luminance are calculated after each Operation. The results are more acurate on CPU
    // Mode. This is a downside of Loop Fusion. But if the Brightness doesn't change too much, it
    // will still be fine.
    float minL, maxL;

    if (!m_statsValid) {
        // 6.1 Calculate stats
        cv::Mat thumbnail = ImageUtils::createThumbnail(processingSrc);
        cv::Mat hslThumbnail = ColorSpace::convertBGR2HSL(thumbnail);
        auto minMax = ImageUtils::calculateMinMax(hslThumbnail, 2);  // 2 = Luminance

        // 6.2 Cache stats
        m_cachedMinL = std::get<0>(minMax);
        m_cachedMaxL = std::get<1>(minMax);
        m_statsValid = true;
    }

    // 7. Use cached values
    minL = m_cachedMinL;
    maxL = m_cachedMaxL;
    float range = maxL - minL;

    // 8. Prepare Parameters (Mapping Sliders -> Factors)
    // Exposure
    float exposure_factor = std::pow(2.0f, m_currentState.exposure);

    // Constrast
    float contrast = 1.0f + (m_currentState.contrast / AdjustContrast::CONTRAST_SCALING_FACTOR);

    // Brightness
    float brightness_factor =
        m_currentState.brightness / AdjustBrightness::BRIGHTNESS_SCALING_FACTOR;

    // Highlight
    float highlight_f = m_currentState.highlight / AdjustHighlight::HIGHLIGHT_SCALING_FACTOR;
    float highlight_under = minL + (range * AdjustHighlight::WEIGHT_RANGE_LOWER);
    float highlight_upper = minL + (range * AdjustHighlight::WEIGHT_RANGE_UPPER);

    // Shadow
    float shadow_f = m_currentState.shadow / AdjustShadow::SHADOW_SCALING_FACTOR;
    float shadow_under = minL + (range * AdjustShadow::WEIGHT_RANGE_LOWER);
    float shadow_upper = minL + (range * AdjustShadow::WEIGHT_RANGE_UPPER);

    // White
    float white_f = m_currentState.white / AdjustWhite::WHITE_SCALING_FACTOR;
    float white_under = minL + (range * AdjustWhite::WEIGHT_RANGE_LOWER);
    float white_upper = minL + (range * AdjustWhite::WEIGHT_RANGE_UPPER);

    // Black
    float black_f = m_currentState.black / AdjustBlack::BLACK_SCALING_FACTOR;
    float black_lower = minL + (range * AdjustBlack::LOWER_THRESHOLD_PERCENT);
    float black_upper = minL + (range * AdjustBlack::UPPER_THRESHOLD_PERCENT);

    // Saturation
    float sat_factor =
        1.0f + (m_currentState.saturation / AdjustSaturation::SATURATION_SCALING_FACTOR);

    // Vibrance
    float vibrance_f = m_currentState.vibrance / AdjustVibrance::VIBRANCE_SCALING_FACTOR;

    // Tint Magenta
    float t_mag = 1.0f - (m_currentState.tintMagenta / TintMagenta::TINT_SCALING_FACTOR);

    // White Balance
    float wb_red = 1.0f + (m_currentState.temp / WhiteBalance::WHITE_BALANCE_FACTOR);
    float wb_blue = 1.0f - (m_currentState.temp / WhiteBalance::WHITE_BALANCE_FACTOR);

    // Detail -
    float sharpen_amt = m_currentState.sharpen / Sharpen::SHARPEN_SCALING_FACTOR;
    float clarity_amt = m_currentState.clarity / Clarity::CLARITY_SCALING_FACTOR;

    // 3. Prepare Buffers (Planar BGR)
    // Generator expects BGR planar input as 16-bit (uint16_t).
    Halide::Runtime::Buffer<uint16_t> inputPlanar(processingSrc.cols, processingSrc.rows, 3);
    Halide::Runtime::Buffer<uint16_t> outputPlanar(processingSrc.cols, processingSrc.rows, 3);

    // 4. Copy Interleaved BGR -> Planar BGR

    // clang-format off
    // Use parallel loop for speed
    #pragma omp parallel for
    // clang-format on
    for (int y = 0; y < processingSrc.rows; y++) {
        const cv::Vec3w* rowPtr = processingSrc.ptr<cv::Vec3w>(y);
        for (int x = 0; x < processingSrc.cols; x++) {
            const cv::Vec3w& pixel = rowPtr[x];
            inputPlanar(x, y, 0) = pixel[0];  // B
            inputPlanar(x, y, 1) = pixel[1];  // G
            inputPlanar(x, y, 2) = pixel[2];  // R
        }
    }

    // 6. Call AOT Function
    int err = photo_adjustment(
        (struct halide_buffer_t*)(inputPlanar.raw_buffer()), (float)exposure_factor,
        (float)contrast, (float)brightness_factor, (float)minL, (float)maxL, (float)highlight_f,
        (float)highlight_under, (float)highlight_upper, (float)shadow_f, (float)shadow_under,
        (float)shadow_upper, (float)white_f, (float)white_under, (float)white_upper, (float)black_f,
        (float)black_lower, (float)black_upper, (float)sat_factor, (float)vibrance_f, (float)t_mag,
        (float)wb_red, (float)wb_blue, (float)sharpen_amt, (float)clarity_amt, 1.0f, 0.1f, 0.0f,
        (struct halide_buffer_t*)(outputPlanar.raw_buffer()));

    if (err != 0) {
        std::cerr << "❌ AOT Pipeline Failed: " << err << std::endl;

        // Fallback to CPU Sequential
        bool originalMode = m_pipeline.isFusionMode();
        m_pipeline.setFusionMode(false);  // Force CPU
        cv::Mat result = m_pipeline.process();
        m_pipeline.setFusionMode(originalMode);  // Restore
        return result;
    }

    // 7. Explicitly wait for GPU to finish before reading data
    outputPlanar.device_sync();

    // 8. Convert Output Planar -> Interleaved BGR (CV_16UC3)
    cv::Mat dst(processingSrc.size(), processingSrc.type());

    // clang-format off
    // 9. Convert Output Planar -> Interleaved BGR (CV_16UC3)
    #pragma omp parallel for
    // clang-format on
    for (int y = 0; y < processingSrc.rows; y++) {
        cv::Vec3w* rowPtr = dst.ptr<cv::Vec3w>(y);
        for (int x = 0; x < processingSrc.cols; x++) {
            cv::Vec3w& pixel = rowPtr[x];
            pixel[0] = outputPlanar(x, y, 0);  // B
            pixel[1] = outputPlanar(x, y, 1);  // G
            pixel[2] = outputPlanar(x, y, 2);  // R
        }
    }

    // If original was 8-bit, Convert back to 8-bit
    if (src.depth() == CV_8U) {
        dst.convertTo(dst, CV_8U, 1.0 / 256.0);
    }

    // 10. CPU Operations (Hybrid Pipeline)
    // Applied in order: Crop -> Denoise -> Resize -> Rotate -> Flip

    cv::Mat finalImg = dst;

    // Crop
    if (!m_currentState.cropRect.empty()) {
        Crop cropOp(m_currentState.cropRect);
        finalImg = cropOp.apply(finalImg);
    }

    // Effects (Dynamic via Registry)
    for (const auto& effectName : m_currentState.activeEffects) {
        auto op = OperationRegistry::getInstance().createFilter(effectName);
        if (op) {
            finalImg = op->apply(finalImg);
        }
    }

    // Denoise
    if (std::abs(m_currentState.denoise) > 0.001f) {
        Denoise denoiseOp(static_cast<int>(m_currentState.denoise));
        finalImg = denoiseOp.apply(finalImg);
    }

    // Resize
    if ((m_currentState.resizeWidth > 0 || m_currentState.resizeHeight > 0) ||
        m_currentState.resizeRatio > 0.0f) {
        // Sanity Check: If Ratio AND Width are set -> Ambiguous!
        // Strategy: Prioritize Ratio + Height (User request to avoid ambiguity)
        if (m_currentState.resizeRatio > 0.0f && m_currentState.resizeWidth > 0) {
            std::cerr
                << "⚠️ Warning: both resizeRatio and resizeWidth are set. Ignoring resizeWidth!"
                << std::endl;
        }

        // Option A: Height + Ratio
        if (m_currentState.resizeRatio > 0.0f && m_currentState.resizeHeight > 0) {
            ResizeImage resizeOp(static_cast<unsigned int>(m_currentState.resizeHeight),
                                 static_cast<double>(m_currentState.resizeRatio));
            finalImg = resizeOp.apply(finalImg);
        }
        // Option B: Width + Height
        else {
            unsigned int w = static_cast<unsigned int>(m_currentState.resizeWidth);
            unsigned int h = static_cast<unsigned int>(m_currentState.resizeHeight);

            // Aspect Ratio Logic: If one is 0, calculate based on original
            if (w == 0 && h > 0) {
                w = static_cast<unsigned int>(finalImg.cols * ((float)h / finalImg.rows));
            } else if (h == 0 && w > 0) {
                h = static_cast<unsigned int>(finalImg.rows * ((float)w / finalImg.cols));
            }

            // Constructor expects (height, width)
            ResizeImage resizeOp(h, w);
            finalImg = resizeOp.apply(finalImg);
        }
    }

    // Rotate
    if (std::abs(m_currentState.rotation) > 0.001f) {
        cv::Rect roi = m_currentState.cropRect;  // Rotate needs ROI to cut out the black corners
        Rotate rotateOp(m_currentState.rotation, roi);
        finalImg = rotateOp.apply(finalImg);
    }

    // Flip
    if (m_currentState.flip != -1) {
        Flip flipOp(m_currentState.flip);
        finalImg = flipOp.apply(finalImg);
    }

    return finalImg;
#else
    return m_pipeline.process();
#endif
}

void ImageController::rebuildPipeline(const ImageState& state) {
    // Clear existing pipeline
    m_pipeline.clearUndoHistory();
    m_pipeline.clearOperations();

    // --- PIPELINE ORDER DEFINITION ---
    // The order here defines the AOT kernel structure.
    // Standard photographic pipeline:
    // 1. Crop --> to reduce the amount of pixels processed
    // 2. White Balance (Temp/Tint)
    // 3. Exposure / Tone Mapping (Highlight, Shadow, Whites, Blacks)
    // 4. Contrast
    // 5. Color (Saturation, Vibrance)
    // 6. Denoise
    // 7. Resize
    // 8. Rotate
    // 9. Flip

    // 1. Crop
    if (!state.cropRect.empty()) {
        m_pipeline.addOperation(std::make_shared<Crop>(state.cropRect));
    }

    // Effects (Dynamic via Registry)
    for (const auto& effectName : state.activeEffects) {
        auto op = OperationRegistry::getInstance().createFilter(effectName);
        if (op) {
            m_pipeline.addOperation(op);
        }
    }

    // =========================================================================
    // RGB Block (Match AOT Order: Exposure -> WB -> Tint)
    // =========================================================================

    // 2. Exposure
    if (std::abs(state.exposure) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustExposure>(state.exposure));
    }

    // 3. White Balance
    if (std::abs(state.temp) > 0.001f || std::abs(state.tint) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<WhiteBalance>(state.temp));
    }

    // 4. Tint Magenta
    if (std::abs(state.tintMagenta) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<TintMagenta>(state.tintMagenta));
    }

    // =========================================================================
    // 2. HSL Block (Tone & Color)
    // =========================================================================

    // 5. Brightness
    if (std::abs(state.brightness) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustBrightness>(state.brightness));
    }

    // 6. Tone Mapping (Highlight, Shadow, White, Black)
    if (std::abs(state.highlight) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustHighlight>(state.highlight));
    }

    // 7. Shadow
    if (std::abs(state.shadow) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustShadow>(state.shadow));
    }

    // 8. White
    if (std::abs(state.white) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustWhite>(state.white));
    }

    // 9. Black
    if (std::abs(state.black) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustBlack>(state.black));
    }

    // 10. Contrast
    if (std::abs(state.contrast) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustContrast>(state.contrast));
    }

    // 11. Saturation
    if (std::abs(state.saturation) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustSaturation>(state.saturation));
    }

    // 12. Vibrance
    if (std::abs(state.vibrance) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustVibrance>(state.vibrance));
    }

    // 13. Sharpen
    if (std::abs(state.sharpen) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<Sharpen>(static_cast<int>(state.sharpen)));
    }

    // 14. Clarity
    if (std::abs(state.clarity) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<Clarity>(static_cast<int>(state.clarity)));
    }

    // =========================================================================
    // Geometry
    // =========================================================================

    // 14. Denoise
    if (std::abs(state.denoise) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<Denoise>(static_cast<int>(state.denoise)));
    }

    // 15. Resize
    if (std::abs(state.resizeRatio) > 0.001f || std::abs(state.resizeWidth) > 0 ||
        std::abs(state.resizeHeight) > 0) {
        // Need current image dimensions to calculate target size if dynamic
        cv::Mat currentImg = m_pipeline.getImg();
        int inputW = currentImg.empty() ? 0 : currentImg.cols;
        int inputH = currentImg.empty() ? 0 : currentImg.rows;

        // If Crop is active, resize applies to the cropped area!
        if (!state.cropRect.empty()) {
            inputW = state.cropRect.width;
            inputH = state.cropRect.height;
        }

        // Logic adapted from original process():
        if (state.resizeRatio > 0.0f && state.resizeHeight > 0) {
            // Option A: Height + Ratio
            m_pipeline.addOperation(
                std::make_shared<ResizeImage>(static_cast<unsigned int>(state.resizeHeight),
                                              static_cast<double>(state.resizeRatio)));
        } else {
            // Option B: Width + Height
            unsigned int w = static_cast<unsigned int>(state.resizeWidth);
            unsigned int h = static_cast<unsigned int>(state.resizeHeight);

            // Aspect Ratio Logic if image is available
            if (inputW > 0 && inputH > 0) {
                if (w == 0 && h > 0) {
                    w = static_cast<unsigned int>(inputW * ((float)h / inputH));
                } else if (h == 0 && w > 0) {
                    h = static_cast<unsigned int>(inputH * ((float)w / inputW));
                }
            }

            // Only add if we have valid dimensions
            if (w > 0 && h > 0) {
                m_pipeline.addOperation(std::make_shared<ResizeImage>(h, w));
            }
        }
    }

    // 16. Rotate
    if (std::abs(state.rotation) > 0.001f) {
        // Rotate needs an ROI. Use cropRect if present, else full image
        cv::Rect roi = state.cropRect;
        m_pipeline.addOperation(std::make_shared<Rotate>(state.rotation, roi));
    }

    // 17. Flip
    if (state.flip != -1) {
        m_pipeline.addOperation(std::make_shared<Flip>(state.flip));
    }
}
