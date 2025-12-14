#ifndef HALIDE_BUILD_GRAPH_H
#define HALIDE_BUILD_GRAPH_H

#include "../utils/gaussian.h"
#include "../utils/halide_image_utils.h"
#include "Halide.h"

/**
 * @brief Central repository for Image Processing Algorithms in Halide.
 *
 * Design Rule:
 * 1. Functions accept Halide::Expr (Pixel Value) or Halide::Func (Image).
 * 2. Functions DO NOT perform Color Space Conversions.
 *    - Example: apply_highlight expects Luminance (L), not RGB.
 * 3. Functions are static and stateless.
 */
class HalideBuildGraph {
   public:
    // =========================================================================
    // RGB Domain Operations
    // =========================================================================

    /**
     * @brief Applies Exposure (Gain) to RGB values.
     * The input image is expected to be in the range [0, 1].
     * @param srcImg srcImg RGB Func
     * @param factor Exposure factor (e.g. 2^ev)
     * @return Halide::Func dstImg RGB Values in the range [0, 1]
     */
    static Halide::Func apply_exposure(Halide::Func srcImg, Halide::Expr factor) {
        Halide::Var x("x"), y("y"), c("c");
        Halide::Func dstImg("exposure_logic");
        dstImg(x, y, c) = Halide::clamp(srcImg(x, y, c) * factor, 0.0f, 1.0f);
        return dstImg;
    }

    /**
     * @brief Applies White Balance (Channel Gains)
     * The input image is expected to be in the range [0, 1].
     * @param srcImg srcImg RGB Func
     * @param factorR Red Channel Gain
     * @param factorB Blue Channel Gain
     * @return Halide::Func dstImg RGB Values in the range [0, 1]
     */
    static Halide::Func apply_white_balance(Halide::Func srcImg, Halide::Expr factorR,
                                            Halide::Expr factorB) {
        Halide::Var x("x"), y("y"), c("c");
        Halide::Func dstImg("wb_logic");
        // Channel 0 = Blue, 1 = Green, 2 = Red (OpenCV default)
        // Adjust Blue and Red. Green stays 1.0
        Halide::Expr val = srcImg(x, y, c);
        dstImg(x, y, c) =
            Halide::select(c == 0, val * factorB, Halide::select(c == 2, val * factorR, val));
        return dstImg;
    }

    /**
     * @brief Applies Tint (Green Correction)
     * Actually "Tint" typically adjusts Green channel relative to RB.
     */
    static Halide::Func apply_tint(Halide::Func srcImg, Halide::Expr factorG) {
        Halide::Var x("x"), y("y"), c("c");
        Halide::Func dstImg("tint_logic");
        Halide::Expr val = srcImg(x, y, c);
        // Adjust Green Channel (c == 1)
        dstImg(x, y, c) = Halide::select(c == 1, val * factorG, val);
        return dstImg;
    }

    // =========================================================================
    // HSL Domain Operations - LUMINANCE (L)
    // =========================================================================

    /**
     * @brief Applies Brightness to Luminance channel.
     * Formula: clamp(L * factor, 0.0, 1.0)
     */
    static Halide::Expr apply_brightness_L(Halide::Expr L, Halide::Expr factor) {
        return Halide::clamp(L * factor, 0.0f, 1.0f);
    }

    /**
     * @brief Applies Highlight Adjustment (Reduces or Boosts bright areas)
     * Uses HalideImageUtils::calculateBrightWeight
     */
    static Halide::Expr apply_highlight_L(Halide::Expr L, Halide::Expr factor, Halide::Expr under,
                                          Halide::Expr upper) {
        Halide::Expr weight = HalideImageUtils::calculateBrightWeight(L, under, upper);
        Halide::Expr delta = weight * factor;
        return Halide::clamp(L + delta, 0.0f, 1.0f);
    }

    /**
     * @brief Applies Shadow Adjustment (Brightens dark areas)
     * Uses HalideImageUtils::calculateDarkWeight
     */
    static Halide::Expr apply_shadow_L(Halide::Expr L, Halide::Expr factor, Halide::Expr under,
                                       Halide::Expr upper) {
        Halide::Expr weight = HalideImageUtils::calculateDarkWeight(L, under, upper);
        Halide::Expr delta = weight * factor;
        return Halide::clamp(L + delta, 0.0f, 1.0f);
    }

    /**
     * @brief Applies White Adjustment (Extreme Highlights)
     * Same logic as Highlight but typically different parameters
     */
    static Halide::Expr apply_white_L(Halide::Expr L, Halide::Expr factor, Halide::Expr under,
                                      Halide::Expr upper) {
        return apply_highlight_L(L, factor, under, upper);
    }

    /**
     * @brief Applies Black Adjustment (Deep Shadows)
     * Same logic as Shadow but typically different parameters
     */
    static Halide::Expr apply_black_L(Halide::Expr L, Halide::Expr factor, Halide::Expr under,
                                      Halide::Expr upper) {
        return apply_shadow_L(L, factor, under, upper);
    }

    /**
     * @brief Applies Contrast
     * Formula: (L - 0.5) * factor + 0.5
     */
    static Halide::Expr apply_contrast_L(Halide::Expr L, Halide::Expr factor) {
        return (L - 0.5f) * factor + 0.5f;
    }

    // =========================================================================
    // HSL Domain Operations - SATURATION (S)
    // =========================================================================

    /**
     * @brief Applies Saturation
     * Formula: clamp(S * factor, 0.0, 1.0)
     */
    static Halide::Expr apply_saturation_S(Halide::Expr S, Halide::Expr factor) {
        return Halide::clamp(S * factor, 0.0f, 1.0f);
    }

    /**
     * @brief Applies Vibrance (Selective Saturation)
     * Boosts lower saturation pixels more than high saturation ones.
     */
    static Halide::Expr apply_vibrance_S(Halide::Expr S, Halide::Expr factor, Halide::Expr low,
                                         Halide::Expr high) {
        Halide::Expr weight = HalideImageUtils::calculateDarkWeight(S, low, high);
        return Halide::clamp(S + weight * factor, 0.0f, 1.0f);
    }
    // =========================================================================
    // Spatial Domain Operations (Sharpen, Clarity, etc.)
    // =========================================================================

    /**
     * @brief Applies Sharpening (Unsharp Mask)
     * Formula: Original + (Original - Blurred) * Amount
     */
    static Halide::Func apply_sharpen(Halide::Func srcImg, Halide::Expr amount, Halide::Expr width,
                                      Halide::Expr height) {
        Halide::Var x("x"), y("y"), c("c");

        // Create Blurred Version (Sigma = 1.0 for Sharpen standard)
        Halide::Func blurred = GaussianFilter::createHalideGraph(srcImg, 1.0f, width, height);

        Halide::Func dstImg("sharpen_logic");
        Halide::Expr valOrig = srcImg(x, y, c);
        Halide::Expr valBlur = blurred(x, y, c);
        Halide::Expr diff = valOrig - valBlur;

        dstImg(x, y, c) = valOrig + diff * amount;
        return dstImg;
    }

    /**
     * @brief Applies Clarity (Local Contrast)
     * Formula: Original + (Original - Blurred) * Amount
     * Similar to Sharpen but with larger radius (Sigma = 2.0 or more)
     */
    static Halide::Func apply_clarity(Halide::Func srcImg, Halide::Expr amount, Halide::Expr width,
                                      Halide::Expr height) {
        Halide::Var x("x"), y("y"), c("c");

        // Create Blurred Version (Sigma = 2.0 for Clarity standard)
        Halide::Func blurred = GaussianFilter::createHalideGraph(srcImg, 2.0f, width, height);

        Halide::Func dstImg("clarity_logic");
        Halide::Expr valOrig = srcImg(x, y, c);
        Halide::Expr valBlur = blurred(x, y, c);
        Halide::Expr diff = valOrig - valBlur;

        dstImg(x, y, c) = valOrig + diff * amount;
        return dstImg;
    }
};

#endif  // HALIDE_BUILD_GRAPH_H
