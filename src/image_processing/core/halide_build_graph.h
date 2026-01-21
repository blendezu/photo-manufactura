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
 * 2. Functions DO NOT perform Color Space Conversions within the function:
 *    - Give input in the expected color space and return the same color space.
 *    - Example: apply_highlight expects Luminance (L), not RGB.
 * 3. Functions are static and stateless.
 */
class HalideBuildGraph {
   public:
    // =========================================================================
    // Operations using RGB Color Space [0, 1]
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
     * @return dstImg RGB Values in the range [0, 1]
     */
    static Halide::Func apply_white_balance(Halide::Func srcImg, Halide::Expr factorR,
                                            Halide::Expr factorB) {
        Halide::Var x("x"), y("y"), c("c");
        Halide::Func dstImg("wb_logic");
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
        dstImg(x, y, c) = Halide::select(c == 1, val * factorG, val);
        return dstImg;
    }

    // =========================================================================
    // Operations using Luminance (L) Channel
    // =========================================================================

    /**
     * @brief Applies Brightness to Luminance channel.
     * Formula: clamp(L * factor, 0.0, 1.0)
     */
    static Halide::Expr apply_brightness_L(Halide::Expr L, Halide::Expr factor, Halide::Expr minL,
                                           Halide::Expr maxL) {
        Halide::Expr range = maxL - minL + 0.0001f;
        Halide::Expr l_norm = (L - minL) / range;
        // Midtone curve: 4 * x * (1-x)
        Halide::Expr weight = 4.0f * l_norm * (1.0f - l_norm);
        weight = Halide::max(0.0f, weight);
        Halide::Expr delta = weight * factor;
        return Halide::clamp(L + delta, 0.0f, 1.0f);
    }

    /**
     * @brief Applies Highlight Adjustment (Reduces or Boosts bright areas)
     * Uses a smooth curve to apply highlight by using weight from
     * HalideImageUtils::calculateBrightWeight for
     * @param L Luminance Channel
     * @param factor Highlight Factor
     * @param under Lower Threshold for Highlight
     * @param upper Upper Threshold for Highlight
     * @return Adjusted Luminance Channel
     */
    static Halide::Expr apply_highlight_L(Halide::Expr L, Halide::Expr factor, Halide::Expr under,
                                          Halide::Expr upper) {
        Halide::Expr weight = HalideImageUtils::calculateBrightWeight(L, under, upper);
        Halide::Expr deltaL = weight * factor;
        return Halide::clamp(L + deltaL, 0.0f, 1.0f);
    }

    /**
     * @brief Applies Shadow Adjustment (Brightens dark areas)
     * Uses a smooth curve to apply shadow by using weight from
     * HalideImageUtils::calculateDarkWeight
     * @param L Luminance Channel
     * @param factor Shadow Factor
     * @param under Lower Threshold for Shadow
     * @param upper Upper Threshold for Shadow
     * @return Adjusted Luminance Channel
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
     * @brief Applies Contrast by using the formula: (L - 0.5) * factor + 0.5
     * @param L Luminance Channel
     * @param factor Contrast Factor
     * @return Adjusted Luminance Channel
     */
    static Halide::Expr apply_contrast_L(Halide::Expr L, Halide::Expr factor) {
        return (L - 0.5f) * factor + 0.5f;
    }

    // =========================================================================
    // Operations using Saturation (S) Channel
    // =========================================================================

    /**
     * @brief Applies Saturation by using the formula: clamp(S * factor, 0.0, 1.0)
     * @param S Saturation Channel
     * @param factor Saturation Factor
     * @return Adjusted Saturation Channel
     */
    static Halide::Expr apply_saturation_S(Halide::Expr S, Halide::Expr factor) {
        return Halide::clamp(S * factor, 0.0f, 1.0f);
    }

    /**
     * @brief Applies Vibrance (Selective Saturation)
     * Boosts lower saturation pixels more than high saturation ones.
     * Uses a smooth curve to apply vibrance by using weight from
     * HalideImageUtils::calculateDarkWeight
     * @param S Saturation Channel
     * @param factor Vibrance Factor
     * @param low Lower Threshold for Vibrance
     * @param high Upper Threshold for Vibrance
     * @return Adjusted Saturation Channel
     */
    static Halide::Expr apply_vibrance_S(Halide::Expr S, Halide::Expr factor, Halide::Expr low,
                                         Halide::Expr high) {
        Halide::Expr weight = HalideImageUtils::calculateDarkWeight(S, low, high);
        return Halide::clamp(S + weight * factor, 0.0f, 1.0f);
    }

    // =========================================================================
    // Operations using Spatial Domain (Sharpen, Clarity, etc.)
    // =========================================================================

    /**
     * @brief Applies Sharpening (Unsharp Mask)
     * Formula: Original + (Original - Blurred) * Amount
     * @param srcImg Source Image
     * @param amount Amount of Sharpening
     * @param width Image Width
     * @param height Image Height
     * @return Sharpened Image
     */
    static Halide::Func apply_sharpen(Halide::Func srcImg, Halide::Expr amount, Halide::Expr width,
                                      Halide::Expr height) {
        Halide::Var x("x"), y("y"), c("c");

        // Create Blurred Version (Sigma = 1.0 for Sharpen standard)
        Halide::Func blurredImg = GaussianFilter::createHalideGraph(srcImg, 1.0f, width, height);

        Halide::Func dstImg("sharpen_logic");
        Halide::Expr valOrig = srcImg(x, y, c);
        Halide::Expr valBlur = blurredImg(x, y, c);
        Halide::Expr diffImg = valOrig - valBlur;

        dstImg(x, y, c) = valOrig + diffImg * amount;
        return dstImg;
    }

    /**
     * @brief Applies Clarity (Local Contrast)
     * Formula: Original + (Original - Blurred) * Amount
     * Similar to Sharpen but with larger radius (Sigma = 2.0 or more)
     * @param srcImg Source Image
     * @param amount Amount of Clarity
     * @param width Image Width
     * @param height Image Height
     * @return Clarity Image
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
