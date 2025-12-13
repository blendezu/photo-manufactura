
#include "../utils/bilateral_filter.h"
#include "../utils/gaussian.h"
#include "../utils/halide_color_space.h"
#include "../utils/halide_image_utils.h"
#include "../utils/image_algorithms.h"
#include "Halide.h"

using namespace Halide;

class PhotoAdjustmentGenerator : public Halide::Generator<PhotoAdjustmentGenerator> {
   public:
    // Inputs
    Input<Buffer<uint16_t>> input{"input", 3};  // HWC 16-bit image

    // --- Light Parameters ---
    Input<float> exposure_factor{"exposure_factor"};      // pow(2, exposure)
    Input<float> contrast_factor{"contrast_factor"};      // 1.0 + contrast / 50.0
    Input<float> brightness_factor{"brightness_factor"};  // 1.0 + brightness / 50.0

    // Highlight / Shadow / White / Black (Require Stat-derived params)
    Input<float> highlight_factor{"highlight_factor"};
    Input<float> highlight_under{"highlight_under"};
    Input<float> highlight_upper{"highlight_upper"};

    Input<float> shadow_factor{"shadow_factor"};
    Input<float> shadow_under{"shadow_under"};
    Input<float> shadow_upper{"shadow_upper"};

    Input<float> white_factor{"white_factor"};
    Input<float> white_under{"white_under"};
    Input<float> white_upper{"white_upper"};

    Input<float> black_factor{"black_factor"};
    Input<float> black_lower{"black_lower"};
    Input<float> black_upper{"black_upper"};

    // --- Color Parameters ---
    Input<float> saturation_factor{"saturation_factor"};  // 1.0 + sat / 50.0
    Input<float> vibrance_factor{"vibrance_factor"};      // vibrance / 50.0

    // Temp/Tint (Matrix mult) - Placeholder for now if not fully implemented in Utils
    // We assume strict BGR->HSL->Adjust->BGR flow for now as per current Ops.

    Input<float> tint_magenta_factor{"tint_magenta_factor"};  // 1 - tint / 50.0
    // White Balance
    Input<float> wb_factor_r{"wb_factor_r"};
    Input<float> wb_factor_b{"wb_factor_b"};

    // --- Detail Parameters ---
    // Sharpen
    Input<float> sharpen_amount{"sharpen_amount"};  // derived from strength

    // Clarity
    Input<float> clarity_amount{"clarity_amount"};  // derived from strength

    // Denoise Params (Moved to CPU - Removed from AOT)
    // Input<float> denoise_sigma_spatial{"denoise_sigma_spatial"};
    // Input<float> denoise_sigma_range{"denoise_sigma_range"};
    // Input<float> denoise_blend{"denoise_blend"};

    // Geometry
    // Usually handled by wrapping logic or separate Affine transforms.
    // Crop/Rotate might be complex in a single pipeline if dimensions change.
    // For AOT, usually we stick to pixel-wise or window-based ops that keep size or clear mapping.
    // Crop/Rotate often done separately or as final stage.
    // Let's stick to Color/Light/Detail for now.

    // Outputs
    Output<Buffer<uint16_t>> output{"output", 3};

    void generate() {
        // --- Estimates for Auto-Scheduler ---
        // Input Buffer
        input.dim(0).set_estimate(0, 6000);
        input.dim(1).set_estimate(0, 4000);
        input.dim(2).set_estimate(0, 3);

        // Light Params
        exposure_factor.set_estimate(1.0f);
        contrast_factor.set_estimate(1.0f);
        brightness_factor.set_estimate(1.0f);

        highlight_factor.set_estimate(0.0f);
        highlight_under.set_estimate(0.5f);
        highlight_upper.set_estimate(1.0f);

        shadow_factor.set_estimate(0.0f);
        shadow_under.set_estimate(0.0f);
        shadow_upper.set_estimate(0.5f);

        white_factor.set_estimate(0.0f);
        white_under.set_estimate(0.8f);
        white_upper.set_estimate(1.0f);

        black_factor.set_estimate(0.0f);
        black_lower.set_estimate(0.0f);
        black_upper.set_estimate(0.2f);

        // Color Params
        saturation_factor.set_estimate(1.0f);
        vibrance_factor.set_estimate(1.0f);
        tint_magenta_factor.set_estimate(1.0f);
        wb_factor_r.set_estimate(1.0f);
        wb_factor_b.set_estimate(1.0f);

        // Detail Params
        sharpen_amount.set_estimate(0.0f);
        clarity_amount.set_estimate(0.0f);

        // Denoise Params Estimates (Removed)
        // denoise_sigma_spatial.set_estimate(1.0f);
        // denoise_sigma_range.set_estimate(0.1f);
        // denoise_blend.set_estimate(0.0f);

        Var x("x"), y("y"), c("c");

        // 1. Cast Input to Float (0..1)
        // Assume HWC input.
        Func in_f = Halide::BoundaryConditions::repeat_edge(input);  // Safe access
        // Ideally we work on bounded domain.

        Expr r_norm = 1.0f / 65535.0f;
        Expr val = cast<float>(in_f(x, y, c)) * r_norm;

        // We need to work with full tuple or channel-selected funcs?
        // Most ops work on Func(x,y,c).

        Func f("f");
        f(x, y, c) = val;

        // --- Sequence of Operations (Matches ImageController order) ---

        // 1. Exposure
        // output = input * factor
        Func f_exposure;
        f_exposure(x, y, c) = f(x, y, c) * exposure_factor;

        // 2. Highlight / Shadow / White / Black
        // These are complex ops defined in HalideImageUtils/Ops.
        // We need to inline the logic or allow calling the logic.
        // Logic in Ops:
        //   Convert to HSL (or Luminance), calculate weight, add delta, clamp.
        //   We can chain them.

        // To avoid massive code duplication, we really should have refactored logic into Utils.
        // But for this task, I will instantiate the logic here using the Utils.

        // All these ops follow: BGR -> HSL -> Adjust L -> BGR.
        // To be efficient, we should convert to HSL ONCE, do all L-adjustments, then back.
        // BUT, the current JIT pipeline is sequential (Op1 -> Op2 -> Op3), converting back and
        // forth? Let's check: AdjustExposure: RGB Highlight: RGB->HSL->Adjust->RGB. Shadow:
        // RGB->HSL->Adjust->RGB. If we strictly follow current component implementation, we do
        // multiple conversions. Optimizing to single HSL conversion is better AOT but changes
        // behavior slightly (color shifts?). Let's stick to sequential functional composition for
        // correctness first.

        Func current = f_exposure;

        // --- MATCH JIT ORDER (ImageController::rebuildPipeline) ---
        // This ensures visual parity between AOT and JIT paths.

        // 1. White Balance (Before tone adjustments)
        current = apply_white_balance(current);
        // Manual schedule removed

        // 2. Brightness (Early in chain to affect all subsequent ops)
        current = apply_brightness(current);
        // Manual schedule removed

        // 3-6. Tone Adjustments (Highlight, Shadow, White, Black)
        current = apply_highlight(current);
        // Manual schedule removed

        current = apply_shadow(current);
        // Manual schedule removed

        current = apply_white(current);
        // Manual schedule removed

        current = apply_black(current);
        // Manual schedule removed

        // 7. Contrast
        current = apply_contrast(current);
        // Manual schedule removed

        // 8. Saturation (Before Vibrance to allow desaturation)
        current = apply_saturation(current);
        // Manual schedule removed

        // 9. Vibrance (Selective saturation boost)
        current = apply_vibrance(current);
        // Manual schedule removed for auto-scheduler

        // 10. Tint Magenta (AFTER Saturation, so it can add color to desaturated image)
        current = apply_tint_magenta(current);
        // Manual schedule removed for auto-scheduler

        // Get image dimensions for filters below
        Expr width = input.dim(0).extent();
        Expr height = input.dim(1).extent();

        // --- DENOISE ---
        // Bilateral Filter for Denoising
        // SigmaSpatial ~ 2.0, SigmaRange ~ 0.1
        // We might want to make sigma_spatial parameterizable? Currently strict 2.0 in JIT header
        // default? Using inputs denoise_sigma_spatial / range For performance, we only do this if
        // denoise_blend > 0 ideally, but AOT graph is static. It will compute.

        // --- DENOISE (DISABLED FOR PERFORMANCE - MOVED TO CPU) ---
        // Bilateral Filter is too slow (12s) without specialization.
        // Moved to Hybrid stage in ImageController.

        // Func denoised = BilateralFilter::createHalideGraph(current, denoise_sigma_spatial,
        // denoise_sigma_range, 6, width, height);

        // Blend Denoise
        // Func f_denoise;
        // f_denoise(x, y, c) = (1.0f - denoise_blend) * current(x, y, c) + denoise_blend *
        // denoised(x, y, c);

        // current = f_denoise;

        // Sharpen (Gaussian)
        // Unsharp Mask: Org + (Org - Blur) * Amt
        // Blur Radius ~ 1.0
        Func blurred_sharp = GaussianFilter::createHalideGraph(current, 1.0f, width, height);
        // Manual schedule removed for auto-scheduler

        Func f_sharpen;
        Expr valOrig = current(x, y, c);
        Expr valBlur = blurred_sharp(x, y, c);
        Expr diff = valOrig - valBlur;
        f_sharpen(x, y, c) = valOrig + diff * sharpen_amount;
        current = f_sharpen;
        // Manual schedule removed for auto-scheduler

        // Clarity (Gaussian)
        // Radius ~ 30.0 (Too large for direct convolution in AOT) -> Reduced to 2.0
        Func blurred_clarity = GaussianFilter::createHalideGraph(current, 2.0f, width, height);
        // Manual schedule removed for auto-scheduler

        Func f_clarity;
        Expr valOrig2 = current(x, y, c);
        Expr valBlur2 = blurred_clarity(x, y, c);
        Expr diff2 = valOrig2 - valBlur2;
        f_clarity(x, y, c) = valOrig2 + diff2 * clarity_amount;
        current = f_clarity;
        // Manual schedule removed for auto-scheduler

        // Final Clamp & Cast
        output(x, y, c) = cast<uint16_t>(clamp(current(x, y, c), 0.0f, 1.0f) * 65535.0f);

        // Schedule
        // Using Adams2019 auto-scheduler via CMake directive

        // Estimates are REQUIRED for auto-scheduler
        // SPECIALIZATION Removed due to Auto-Scheduler conflict (Error: f13 invalid location)

        output.dim(0).set_estimate(0, 6000);
        output.dim(1).set_estimate(0, 4000);
        output.dim(2).set_estimate(0, 3);
    }

   private:
    // Helper to encapsulate logic.
    // Each takes a Func (RGB float) and returns Func (RGB float)

    Func apply_highlight(Func in) {
        Var x("x"), y("y"), c("c");
        // Logic from AdjustHighlight::buildGraph
        // 1. Extract BGR
        Expr B = in(x, y, 0);
        Expr G = in(x, y, 1);
        Expr R = in(x, y, 2);

        // 2. To HSL
        std::vector<Expr> hsl = HalideColorSpace::BGR2HSL(B, G, R);
        Expr L = hsl[2];

        // 3. Weight
        Expr weight = HalideImageUtils::calculateBrightWeight(L, highlight_under, highlight_upper);
        Expr delta = weight * highlight_factor;
        Expr newL = clamp(L + delta, 0.0f, 1.0f);

        // 4. Back to BGR
        std::vector<Expr> bgr = HalideColorSpace::HSL2BGR(hsl[0], hsl[1], newL);

        Func out;
        out(x, y, c) = select(c == 0, bgr[0], select(c == 1, bgr[1], bgr[2]));
        return out;
    }

    Func apply_shadow(Func in) {
        Var x("x"), y("y"), c("c");
        Expr B = in(x, y, 0);
        Expr G = in(x, y, 1);
        Expr R = in(x, y, 2);
        std::vector<Expr> hsl = HalideColorSpace::BGR2HSL(B, G, R);
        Expr L = hsl[2];

        Expr weight = HalideImageUtils::calculateDarkWeight(L, shadow_under, shadow_upper);
        Expr delta = weight * shadow_factor;
        Expr newL = clamp(L + delta, 0.0f, 1.0f);

        std::vector<Expr> bgr = HalideColorSpace::HSL2BGR(hsl[0], hsl[1], newL);
        Func out;
        out(x, y, c) = select(c == 0, bgr[0], select(c == 1, bgr[1], bgr[2]));
        return out;
    }

    Func apply_white(Func in) {
        Var x("x"), y("y"), c("c");
        Expr B = in(x, y, 0);
        Expr G = in(x, y, 1);
        Expr R = in(x, y, 2);
        std::vector<Expr> hsl = HalideColorSpace::BGR2HSL(B, G, R);
        Expr L = hsl[2];

        Expr weight = HalideImageUtils::calculateBrightWeight(L, white_under, white_upper);
        Expr delta = weight * white_factor;
        Expr newL = clamp(L + delta, 0.0f, 1.0f);

        std::vector<Expr> bgr = HalideColorSpace::HSL2BGR(hsl[0], hsl[1], newL);
        Func out;
        out(x, y, c) = select(c == 0, bgr[0], select(c == 1, bgr[1], bgr[2]));
        return out;
    }

    Func apply_black(Func in) {
        Var x("x"), y("y"), c("c");
        Expr B = in(x, y, 0);
        Expr G = in(x, y, 1);
        Expr R = in(x, y, 2);
        std::vector<Expr> hsl = HalideColorSpace::BGR2HSL(B, G, R);
        Expr L = hsl[2];

        Expr weight = HalideImageUtils::calculateDarkWeight(L, black_lower, black_upper);
        Expr delta = weight * black_factor;
        Expr newL = clamp(L + delta, 0.0f, 1.0f);

        std::vector<Expr> bgr = HalideColorSpace::HSL2BGR(hsl[0], hsl[1], newL);
        Func out;
        out(x, y, c) = select(c == 0, bgr[0], select(c == 1, bgr[1], bgr[2]));
        return out;
    }

    Func apply_contrast(Func in) {
        Var x("x"), y("y"), c("c");
        // AdjustContrast uses HSL Luminance adjustment too.
        Expr B = in(x, y, 0);
        Expr G = in(x, y, 1);
        Expr R = in(x, y, 2);
        std::vector<Expr> hsl = HalideColorSpace::BGR2HSL(B, G, R);
        Expr H = hsl[0];
        Expr S = hsl[1];
        Expr L = hsl[2];

        // AdjustContrast logic: newL = (currL - 0.5f) * factor + 0.5f
        Expr newL = ImageAlgorithms::apply_contrast(L, contrast_factor);
        // Clamp logic done by HSL2BGR usually or separate?
        // AdjustContrast logic: newL = (currL - 0.5f) * factor + 0.5f

        std::vector<Expr> bgr = HalideColorSpace::HSL2BGR(H, S, newL);
        Func out;
        out(x, y, c) = select(c == 0, bgr[0], select(c == 1, bgr[1], bgr[2]));
        return out;
    }

    Func apply_saturation(Func in) {
        Var x("x"), y("y"), c("c");
        Expr B = in(x, y, 0);
        Expr G = in(x, y, 1);
        Expr R = in(x, y, 2);
        std::vector<Expr> hsl = HalideColorSpace::BGR2HSL(B, G, R);
        Expr H = hsl[0];
        Expr S = hsl[1];
        Expr L = hsl[2];

        Expr newS = ImageAlgorithms::apply_saturation(S, saturation_factor);

        std::vector<Expr> bgr = HalideColorSpace::HSL2BGR(H, newS, L);
        Func out;
        out(x, y, c) = select(c == 0, bgr[0], select(c == 1, bgr[1], bgr[2]));
        return out;
    }

    Func apply_tint_magenta(Func in) {
        Var x("x"), y("y"), c("c");
        Expr B = in(x, y, 0);
        Expr G = in(x, y, 1);
        Expr R = in(x, y, 2);

        // Green correction
        Expr newG = G * tint_magenta_factor;

        Func out;
        out(x, y, c) = select(c == 0, B, select(c == 1, newG, R));
        return out;
    }

    Func apply_brightness(Func in) {
        Var x("x"), y("y"), c("c");
        // Simple multiplicative brightness on all channels
        Expr val = in(x, y, c);
        Expr newVal = ImageAlgorithms::apply_brightness(val, brightness_factor);

        Func out;
        out(x, y, c) = newVal;
        return out;
    }

    Func apply_vibrance(Func in) {
        Var x("x"), y("y"), c("c");
        Expr B = in(x, y, 0);
        Expr G = in(x, y, 1);
        Expr R = in(x, y, 2);
        std::vector<Expr> hsl = HalideColorSpace::BGR2HSL(B, G, R);
        Expr H = hsl[0];
        Expr S = hsl[1];
        Expr L = hsl[2];

        // Vibrance parameters (Thresholds)
        // JIT uses constants: LOWER_THRESHOLD = 0.35f, UPPER_THRESHOLD = 0.45f
        // We can hardcode them here to match, or add Inputs.
        // Let's hardcode for simplicity as they are constants in JIT header.
        Expr low = 0.35f;
        Expr high = 0.45f;

        Expr newS = ImageAlgorithms::apply_vibrance(S, vibrance_factor, low, high);

        std::vector<Expr> bgr = HalideColorSpace::HSL2BGR(H, newS, L);
        Func out;
        out(x, y, c) = select(c == 0, bgr[0], select(c == 1, bgr[1], bgr[2]));
        return out;
    }

    Func apply_white_balance(Func in) {
        Var x("x"), y("y"), c("c");
        Expr B = in(x, y, 0);
        Expr G = in(x, y, 1);
        Expr R = in(x, y, 2);

        Expr newR = R * wb_factor_r;
        Expr newB = B * wb_factor_b;

        Func out;
        out(x, y, c) = select(c == 0, newB, select(c == 1, G, newR));
        return out;
    }
};

HALIDE_REGISTER_GENERATOR(PhotoAdjustmentGenerator, photo_adjustment)

int main(int argc, char** argv) {
    return Halide::Internal::generate_filter_main(argc, argv);
}
