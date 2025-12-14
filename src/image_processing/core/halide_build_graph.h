#ifndef HALIDE_BUILD_GRAPH_H
#define HALIDE_BUILD_GRAPH_H

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
     * @param input Input RGB Func
     * @param factor Exposure factor (e.g. 2^ev)
     */
    static Halide::Func apply_exposure(Halide::Func input, Halide::Expr factor) {
        Halide::Var x("x"), y("y"), c("c");
        Halide::Func out("exposure_logic");
        out(x, y, c) = Halide::clamp(input(x, y, c) * factor, 0.0f,
                                     1.0f);  // Input is assumed to be 0..1 float
        return out;
    }

    /**
     * @brief Applies White Balance (Channel Gains)
     */
    static Halide::Func apply_white_balance(Halide::Func input, Halide::Expr factorR,
                                            Halide::Expr factorB) {
        Halide::Var x("x"), y("y"), c("c");
        Halide::Func out("wb_logic");
        // Channel 0 = Blue, 1 = Green, 2 = Red (OpenCV default)
        // Adjust Blue and Red. Green stays 1.0
        Halide::Expr val = input(x, y, c);
        out(x, y, c) =
            Halide::select(c == 0, val * factorB, Halide::select(c == 2, val * factorR, val));
        return out;
    }

    /**
     * @brief Applies Tint (Green Correction)
     * Actually "Tint" typically adjusts Green channel relative to RB.
     */
    static Halide::Func apply_tint(Halide::Func input, Halide::Expr factorG) {
        Halide::Var x("x"), y("y"), c("c");
        Halide::Func out("tint_logic");
        Halide::Expr val = input(x, y, c);
        // Adjust Green Channel (c == 1)
        out(x, y, c) = Halide::select(c == 1, val * factorG, val);
        return out;
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
};

#endif  // HALIDE_BUILD_GRAPH_H
