
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

    // Denoise
    Input<float> denoise_sigma_spatial{"denoise_sigma_spatial"};
    Input<float> denoise_sigma_range{"denoise_sigma_range"};
    Input<float> denoise_blend{"denoise_blend"};

    // Geometry
    // Usually handled by wrapping logic or separate Affine transforms.
    // Crop/Rotate might be complex in a single pipeline if dimensions change.
    // For AOT, usually we stick to pixel-wise or window-based ops that keep size or clear mapping.
    // Crop/Rotate often done separately or as final stage.
    // Let's stick to Color/Light/Detail for now.

    // Outputs
    Output<Buffer<uint16_t>> output{"output", 3};

    void generate() {
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

        // 2. Highlight / Shadow / White / Black

        // Highlight
        current = apply_highlight(current);
        current.compute_root().parallel(y).vectorize(x, 16);

        // Shadow
        current = apply_shadow(current);
        current.compute_root().parallel(y).vectorize(x, 16);

        // White
        current = apply_white(current);
        current.compute_root().parallel(y).vectorize(x, 16);

        // Black
        current = apply_black(current);
        current.compute_root().parallel(y).vectorize(x, 16);

        // White Balance
        current = apply_white_balance(current);  // [NEW]
        current.compute_root().parallel(y).vectorize(x, 16);

        // Tint Magenta
        current = apply_tint_magenta(current);
        current.compute_root().parallel(y).vectorize(x, 16);

        // Brightness (Multiplicative on Luminance/RGB? JIT does Per-Channel Mul on Color)
        // JIT Brightness logic: val * factor.
        current = apply_brightness(current);
        current.compute_root().parallel(y).vectorize(x, 16);

        // Vibrance (HSL Saturation Adjustment)
        current = apply_vibrance(current);
        current.compute_root().parallel(y).vectorize(x, 16);

        // Contrast (Logic: (L - 0.5)*fac + 0.5)
        current = apply_contrast(current);
        current.compute_root().parallel(y).vectorize(x, 16);

        // Saturation (Logic: HSL, adjust S)
        current = apply_saturation(current);
        current.compute_root().parallel(y).vectorize(x, 16);

        // Denoise (Bilateral) - Computationally heavy
        // Requires Access to neighbors.
        // BilateralFilter::createHalideGraph(input, sigmaS, sigmaR, radius, width, height)
        // We need image dimensions.
        Expr width = input.dim(0).extent();
        Expr height = input.dim(1).extent();

        // Strong Denoise + Blend
        // We need to wrap this logic.
        Func denoised = BilateralFilter::createHalideGraph(current, denoise_sigma_spatial,
                                                           denoise_sigma_range, 1, width, height);
        // Denoise usually needs its own schedule, let's look at BilateralFilter impl.
        // It has internal funcs. We should schedule the output of BilateralFilter.
        denoised.compute_root().parallel(y).vectorize(x, 8);

        Func f_denoised_blend;
        f_denoised_blend(x, y, c) =
            current(x, y, c) * (1.0f - denoise_blend) + denoised(x, y, c) * denoise_blend;

        // Only apply denoise if blend > 0? No, rely on blend factor.
        current = f_denoised_blend;
        current.compute_root().parallel(y).vectorize(x, 16);

        // Sharpen (Gaussian)
        // Unsharp Mask: Org + (Org - Blur) * Amt
        // Blur Radius ~ 1.0
        Func blurred_sharp = GaussianFilter::createHalideGraph(current, 1.0f, width, height);
        blurred_sharp.compute_root().parallel(y).vectorize(x, 16);

        Func f_sharpen;
        Expr valOrig = current(x, y, c);
        Expr valBlur = blurred_sharp(x, y, c);
        Expr diff = valOrig - valBlur;
        f_sharpen(x, y, c) = valOrig + diff * sharpen_amount;
        current = f_sharpen;
        current.compute_root().parallel(y).vectorize(x, 16);

        // Clarity (Gaussian)
        // Radius ~ 30.0 (Too large for direct convolution in AOT) -> Reduced to 2.0
        Func blurred_clarity = GaussianFilter::createHalideGraph(current, 2.0f, width, height);
        blurred_clarity.compute_root().parallel(y).vectorize(x, 16);

        Func f_clarity;
        Expr valOrig2 = current(x, y, c);
        Expr valBlur2 = blurred_clarity(x, y, c);
        Expr diff2 = valOrig2 - valBlur2;
        f_clarity(x, y, c) = valOrig2 + diff2 * clarity_amount;
        current = f_clarity;
        current.compute_root().parallel(y).vectorize(x, 16);

        // Final Clamp & Cast
        output(x, y, c) = cast<uint16_t>(clamp(current(x, y, c), 0.0f, 1.0f) * 65535.0f);

        // Schedule
        // Simple Manual Schedule
        // Compute everything at root for now to ensure correctness
        output.compute_root();
        output.parallel(y);
        output.vectorize(x, 8);
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
