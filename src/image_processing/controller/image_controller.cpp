#include "image_controller.h"

// Include all operations needed
#include <cmath>

// Light
#include "../operations/detail/clarity.h"  // Assuming this is for Whites sliders
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

// Geometry
#include "../operations/denoise/denoise.h"
#include "../operations/geometry/crop.h"
#include "../operations/geometry/flip.h"
#include "../operations/geometry/rotate.h"
#include "../utils/image_resize.h"

ImageController::ImageController() {
    // Default: Use GPU mode for development if available
    m_pipeline.setFusionMode(true);
}

ImageController::~ImageController() {}

void ImageController::loadImage(const cv::Mat& img) {
    m_pipeline.setImg(img);
    m_statsValid = false;  // Invalidate cache on new image
}

void ImageController::setImage(const cv::Mat& img) {
    m_pipeline.setImg(img);
    m_statsValid = false;  // Invalidate cache on new image
}

void ImageController::update(const ImageState& state) {
    // Save state
    m_currentState = state;

    // For JIT Strategy: We rebuild the pipeline based on the fixed order
    rebuildPipeline(state);
}

// AOT Includes
#ifdef AOT_ENABLED
#include "../utils/color_space.h"
#include "../utils/halide_image_utils.h"  // For stats helpers if needed
#include "../utils/image_utils.h"
#include "HalideBuffer.h"
#include "photo_adjustment.h"
#endif

// ... existing includes ...

cv::Mat ImageController::process() {
#ifdef AOT_ENABLED
    // AOT Path

    // Check if GPU Fusion is enabled. If not, fallback to JIT/CPU
    if (!m_pipeline.isFusionMode()) {
        return m_pipeline.process();
    }

    if (m_pipeline.getImg().empty())
        return cv::Mat();

    const cv::Mat& src = m_pipeline.getImg();

    // Ensure 16-bit Input for AOT (High Precision)
    cv::Mat processingSrc;
    if (src.depth() == CV_8U) {
        // Upscale 8-bit to 16-bit
        src.convertTo(processingSrc, CV_16U, 256.0);  // 0-255 -> 0-65280
    } else if (src.depth() == CV_16U) {
        processingSrc = src;
    } else {
        // Fallback for float or other types?
        // Ideally should not happen in this pipeline context, or handle float.
        // Convert to 16-bit for processing
        src.convertTo(processingSrc, CV_16U);
    }

    // Ensure 3 Channels for AOT Pipeline (it assumes Color)
    if (processingSrc.channels() == 1) {
        cv::cvtColor(processingSrc, processingSrc, cv::COLOR_GRAY2BGR);
    } else if (processingSrc.channels() == 4) {
        cv::cvtColor(processingSrc, processingSrc, cv::COLOR_BGRA2BGR);
    }

    // 1. Calculate Stats (Min/Max Luminance) for Dynamic Ranges
    // Cache stats to avoid redundant computation (~50ms saved per call)
    float minL, maxL;

    if (!m_statsValid) {
        // Recalculate stats
        cv::Mat thumbnail = ImageUtils::createThumbnail(processingSrc);
        cv::Mat hslThumbnail = ColorSpace::convertBGR2HSL(thumbnail);
        auto minMax = ImageUtils::calculateMinMax(hslThumbnail, 2);  // 2 = Luminance

        m_cachedMinL = std::get<0>(minMax);
        m_cachedMaxL = std::get<1>(minMax);
        m_statsValid = true;
    }

    // Use cached values
    minL = m_cachedMinL;
    maxL = m_cachedMaxL;
    float range = maxL - minL;

    // 2. Prepare Parameters (Mapping Sliders -> Factors)
    // Exposure
    // AOT Exposure is applied first. JIT Brightness is applied last.
    // To avoid clipping, we ignore Brightness in AOT exposure calc.
    // Exposure Slider is mapped to 2^x
    float exposure_factor = std::pow(2.0f, m_currentState.exposure);

    // Constrast
    // Map Slider (-100..100) -> Factor (0.0 .. 2.0)
    float contrast = 1.0f + (m_currentState.contrast / AdjustContrast::CONTRAST_SCALING_FACTOR);

    // Highlight
    float highlight_f = m_currentState.highlight / AdjustHighlight::HIGHLIGHT_SCALING_FACTOR;
    float high_under = minL + (range * AdjustHighlight::WEIGHT_RANGE_LOWER);
    float high_upper = minL + (range * AdjustHighlight::WEIGHT_RANGE_UPPER);

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
    // Map Slider (-100..100) -> Factor (0.0 .. 2.0)
    float sat_factor =
        1.0f + (m_currentState.saturation / AdjustSaturation::SATURATION_SCALING_FACTOR);

    // Vibrance
    float vibrance_f = m_currentState.vibrance / AdjustVibrance::VIBRANCE_SCALING_FACTOR;

    // Tint Magenta
    float t_mag = 1.0f - (m_currentState.tintMagenta / TintMagenta::TINT_SCALING_FACTOR);

    // White Balance
    // State has temp/tint. Code logic uses them to derive R/B gains.
    // WhiteBalance::prepareParameters logic:
    // R = 1 + temp / FACTOR
    // B = 1 - temp / FACTOR
    // We assume FACTOR is needed. WhiteBalance header doesn't seem to have exposed constant?
    // Let's assume 200.0f from previous learnings/test file if not found.
    // Actually we didn't expose it in WhiteBalance.h. Let's use 200.0f (safe guess from test).
    float wb_red = 1.0f + (m_currentState.temp / 200.0f);
    float wb_blue = 1.0f - (m_currentState.temp / 200.0f);

    // Detail - Scale factors to match JIT logic (sharpen.cpp / clarity.cpp)
    // Sharpen JIT: strength / 50.0f
    // Clarity JIT: strength / 100.0f
    float sharpen_amt = m_currentState.sharpen / 50.0f;
    float clarity_amt = m_currentState.clarity / 100.0f;

    // 3. Prepare Buffers (Planar BGR)
    // Generator expects BGR planar input as 16-bit (uint16_t).
    Halide::Runtime::Buffer<uint16_t> inputPlanar(processingSrc.cols, processingSrc.rows, 3);
    Halide::Runtime::Buffer<uint16_t> outputPlanar(processingSrc.cols, processingSrc.rows, 3);

// Copy Interleaved BGR -> Planar BGR
// Use parallel loop for speed
#pragma omp parallel for
    for (int y = 0; y < processingSrc.rows; y++) {
        for (int x = 0; x < processingSrc.cols; x++) {
            // Use Vec3w for 16-bit pixels
            cv::Vec3w pixel = processingSrc.at<cv::Vec3w>(y, x);
            inputPlanar(x, y, 0) = pixel[0];  // B
            inputPlanar(x, y, 1) = pixel[1];  // G
            inputPlanar(x, y, 2) = pixel[2];  // R
        }
    }

    // Brightness (Multiplicative factor for AOT)
    float brightness_factor =
        1.0f + (m_currentState.brightness / AdjustBrightness::BRIGHTNESS_SCALING_FACTOR);

    // 4. Call AOT Function
    // 'brightness' param (3rd arg) now used as factor.
    int err = photo_adjustment(
        (struct halide_buffer_t*)(inputPlanar.raw_buffer()), (float)exposure_factor,
        (float)contrast, (float)brightness_factor, (float)highlight_f, (float)high_under,
        (float)high_upper, (float)shadow_f, (float)shadow_under, (float)shadow_upper,
        (float)white_f, (float)white_under, (float)white_upper, (float)black_f, (float)black_lower,
        (float)black_upper, (float)sat_factor, (float)vibrance_f, (float)t_mag, (float)wb_red,
        (float)wb_blue, (float)sharpen_amt, (float)clarity_amt, 1.0f, 0.1f, 0.0f,
        (struct halide_buffer_t*)(outputPlanar.raw_buffer()));

    if (err != 0) {
        std::cerr << "❌ AOT Pipeline Failed: " << err << std::endl;

        // Fallback to CPU Sequential (NOT JIT, which is slow on first run)
        bool originalMode = m_pipeline.isFusionMode();
        m_pipeline.setFusionMode(false);  // Force CPU
        cv::Mat result = m_pipeline.process();
        m_pipeline.setFusionMode(originalMode);  // Restore
        return result;
    }

    // 5. Convert Output Planar -> Interleaved BGR (CV_16UC3)
    cv::Mat dst(processingSrc.size(), processingSrc.type());

#pragma omp parallel for
    for (int y = 0; y < processingSrc.rows; y++) {
        for (int x = 0; x < processingSrc.cols; x++) {
            cv::Vec3w& pixel = dst.at<cv::Vec3w>(y, x);
            pixel[0] = outputPlanar(x, y, 0);  // B
            pixel[1] = outputPlanar(x, y, 1);  // G
            pixel[2] = outputPlanar(x, y, 2);  // R
        }
    }

    // If original was 8-bit, Convert back to 8-bit
    if (src.depth() == CV_8U) {
        dst.convertTo(dst, CV_8U, 1.0 / 256.0);
    }

    // 6. CPU Operations (Hybrid Pipeline)
    // Applied in order: Denoise -> Geometry

    // Denoise (CPU fallback for performance reasons)
    if (std::abs(m_currentState.denoise) > 0.001f) {
        Denoise denoiseOp(static_cast<int>(m_currentState.denoise));
        dst = denoiseOp.apply(dst);
    }

    // Geometry
    cv::Mat finalImg = dst;

    // Crop
    if (!m_currentState.cropRect.empty()) {
        Crop cropOp(m_currentState.cropRect);
        finalImg = cropOp.apply(finalImg);
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
        // Option B: Width + Height (Logic I added before)
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
        cv::Rect roi = m_currentState.cropRect;  // Rotate needs ROI context sometimes
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
    // Clear existing pipeline (JIT way)
    m_pipeline.clearUndoHistory();
    m_pipeline.clearOperations();

    // --- PIPELINE ORDER DEFINITION ---
    // The order here defines the AOT kernel structure.
    // Standard photographic pipeline:
    // 1. White Balance (Temp/Tint)
    // 2. Exposure / Tone Mapping (Highlight, Shadow, Whites, Blacks)
    // 3. Contrast
    // 4. Color (Saturation, Vibrance)
    // 4. Color (Saturation, Vibrance)
    // 5. Geometry (Crop, Rotate, Flip)

    // --- GEOMETRY FIRST (CPU OPTIMIZATION) ---
    // If we crop first, all subsequent ops process fewer pixels!
    if (!state.cropRect.empty()) {
        m_pipeline.addOperation(std::make_shared<Crop>(state.cropRect));
    }

    // =========================================================================
    // 1. RGB Block (Match AOT Order: Exposure -> WB -> Tint)
    // =========================================================================

    // 1.1 Exposure
    if (std::abs(state.exposure) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustExposure>(state.exposure));
    }

    // 1.2 White Balance
    if (std::abs(state.temp) > 0.001f || std::abs(state.tint) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<WhiteBalance>(state.temp));
    }

    // 1.3 Tint Magenta
    if (std::abs(state.tintMagenta) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<TintMagenta>(state.tintMagenta));
    }

    // =========================================================================
    // 2. HSL Block (Tone & Color)
    // =========================================================================

    // 2.1 Brightness
    if (std::abs(state.brightness) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustBrightness>(state.brightness));
    }

    // 2.2 Tone Mapping (Highlight, Shadow, White, Black)
    if (std::abs(state.highlight) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustHighlight>(state.highlight));
    }

    if (std::abs(state.shadow) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustShadow>(state.shadow));
    }

    if (std::abs(state.white) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustWhite>(state.white));
    }

    if (std::abs(state.black) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustBlack>(state.black));
    }

    // 2.3 Contrast
    if (std::abs(state.contrast) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustContrast>(state.contrast));
    }

    // 2.4 Color (Saturation, Vibrance)
    if (std::abs(state.saturation) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustSaturation>(state.saturation));
    }

    if (std::abs(state.vibrance) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<AdjustVibrance>(state.vibrance));
    }

    // 5. Detail (Sharpen/Clarity usually applied after color/tone)
    if (std::abs(state.sharpen) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<Sharpen>(static_cast<int>(state.sharpen)));
    }

    if (std::abs(state.clarity) > 0.001f) {
        m_pipeline.addOperation(std::make_shared<Clarity>(static_cast<int>(state.clarity)));
    }

    // 6. Geometry (Applied last in stack, so they sample from the processed image)
    // Crop moved to start for performance!

    if (std::abs(state.rotation) > 0.001f) {
        // Rotate needs an ROI. Use cropRect if present, else full image?
        // But we don't know full image size here easily without accessing pipeline.originalImg.
        // If cropRect is empty, pass empty rect, Rotate logic handles it (defaults to full).
        cv::Rect roi = state.cropRect;
        m_pipeline.addOperation(std::make_shared<Rotate>(state.rotation, roi));
    }

    if (state.flip != -1) {
        m_pipeline.addOperation(std::make_shared<Flip>(state.flip));
    }
}
