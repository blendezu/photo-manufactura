#include "../core/halide_build_graph.h"
#include "../utils/gaussian.h"
#include "../utils/halide_color_space.h"
#include "Halide.h"

class PhotoAdjustmentGenerator : public Halide::Generator<PhotoAdjustmentGenerator> {
   public:
    // Inputs
    Halide::GeneratorInput<Halide::Buffer<uint16_t>> input{"input", 3};  // HWC 16-bit image

    // --- Light Parameters ---
    Halide::GeneratorInput<float> exposure_factor{"exposure_factor"};  // pow(2, exposure)
    Halide::GeneratorInput<float> contrast_factor{"contrast_factor"};  // 1.0 + contrast / 50.0
    Halide::GeneratorInput<float> brightness_factor{"brightness_factor"};

    // Highlight / Shadow / White / Black (Require Stat-derived params)
    Halide::GeneratorInput<float> highlight_factor{"highlight_factor"};
    Halide::GeneratorInput<float> highlight_under{"highlight_under"};
    Halide::GeneratorInput<float> highlight_upper{"highlight_upper"};

    Halide::GeneratorInput<float> shadow_factor{"shadow_factor"};
    Halide::GeneratorInput<float> shadow_under{"shadow_under"};
    Halide::GeneratorInput<float> shadow_upper{"shadow_upper"};

    Halide::GeneratorInput<float> white_factor{"white_factor"};
    Halide::GeneratorInput<float> white_under{"white_under"};
    Halide::GeneratorInput<float> white_upper{"white_upper"};

    Halide::GeneratorInput<float> black_factor{"black_factor"};
    Halide::GeneratorInput<float> black_lower{"black_lower"};
    Halide::GeneratorInput<float> black_upper{"black_upper"};

    // --- Color Parameters ---
    Halide::GeneratorInput<float> saturation_factor{"saturation_factor"};  // 1.0 + sat / 50.0
    Halide::GeneratorInput<float> vibrance_factor{"vibrance_factor"};      // vibrance / 50.0

    // Temp/Tint
    Halide::GeneratorInput<float> tint_magenta_factor{"tint_magenta_factor"};  // 1 - tint / 50.0
    // White Balance
    Halide::GeneratorInput<float> wb_factor_r{"wb_factor_r"};
    Halide::GeneratorInput<float> wb_factor_b{"wb_factor_b"};

    // --- Detail Parameters ---
    // Sharpen
    Halide::GeneratorInput<float> sharpen_amount{"sharpen_amount"};  // derived from strength

    // Clarity
    Halide::GeneratorInput<float> clarity_amount{"clarity_amount"};  // derived from strength

    // Outputs
    Halide::GeneratorOutput<Halide::Buffer<uint16_t>> output{"output", 3};

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

        Halide::Var x("x"), y("y"), c("c");

        // 1. Cast Input to Float (0..1)
        // Assume HWC input.
        Halide::Func in_f = Halide::BoundaryConditions::repeat_edge(input);  // Safe access

        Halide::Expr r_norm = 1.0f / 65535.0f;
        Halide::Expr val = Halide::cast<float>(in_f(x, y, c)) * r_norm;

        Halide::Func f("f");
        f(x, y, c) = val;

        // --- Sequence of Operations ---

        // =====================================================================
        // 1. RGB Block (Linear / Multiplicative)
        // =====================================================================
        // Ops that work best on raw RGB data (Exposure, WB, Tint)

        Halide::Func rgb_ops;
        rgb_ops(x, y, c) = f(x, y, c);

        // 1.1 Exposure
        rgb_ops = HalideBuildGraph::apply_exposure(rgb_ops, exposure_factor);

        // 1.2 White Balance
        rgb_ops = HalideBuildGraph::apply_white_balance(rgb_ops, wb_factor_r, wb_factor_b);

        // 1.3 Tint (Magenta/Green) - Applied in RGB for correctness
        rgb_ops = HalideBuildGraph::apply_tint(rgb_ops, tint_magenta_factor);

        // =====================================================================
        // 2. HSL Block (Fused)
        // =====================================================================
        // All Ops that require HSL. We convert ONCE, process, and convert back ONCE.

        // 2.1 BGR -> HSL Conversion
        Halide::Expr R = rgb_ops(x, y, 2);
        Halide::Expr G = rgb_ops(x, y, 1);
        Halide::Expr B = rgb_ops(x, y, 0);
        std::vector<Halide::Expr> hsl = HalideColorSpace::BGR2HSL(B, G, R);

        Halide::Expr H = hsl[0];
        Halide::Expr S = hsl[1];
        Halide::Expr L = hsl[2];

        // 2.2 Luminance Operations (L)
        // Order: Brightness -> Tone Mapping -> Contrast

        // Brightness (now on L channel to match JIT behavior)
        L = HalideBuildGraph::apply_brightness_L(L, brightness_factor);

        // Tone Mapping
        L = HalideBuildGraph::apply_highlight_L(L, highlight_factor, highlight_under,
                                                highlight_upper);
        L = HalideBuildGraph::apply_shadow_L(L, shadow_factor, shadow_under, shadow_upper);
        L = HalideBuildGraph::apply_white_L(L, white_factor, white_under, white_upper);
        L = HalideBuildGraph::apply_black_L(L, black_factor, black_lower, black_upper);

        // Contrast
        L = HalideBuildGraph::apply_contrast_L(L, contrast_factor);

        // 2.3 Saturation Operations (S)
        // Order: Saturation -> Vibrance

        S = HalideBuildGraph::apply_saturation_S(S, saturation_factor);
        // Vibrance (using JIT constants 0.35 - 0.45)
        S = HalideBuildGraph::apply_vibrance_S(S, vibrance_factor, 0.35f, 0.45f);

        // 2.4 HSL -> BGR Conversion
        std::vector<Halide::Expr> bgr = HalideColorSpace::HSL2BGR(H, S, L);

        Halide::Func current;
        current(x, y, c) = Halide::select(c == 0, bgr[0], Halide::select(c == 1, bgr[1], bgr[2]));

        // =====================================================================
        // 3. Spatial Block (Post-Processing)
        // =====================================================================

        // Get image dimensions for filters below
        Halide::Expr width = input.dim(0).extent();
        Halide::Expr height = input.dim(1).extent();

        // Sharpen (Gaussian)
        Halide::Func blurred_sharp =
            GaussianFilter::createHalideGraph(current, 1.0f, width, height);

        Halide::Func f_sharpen;
        Halide::Expr valOrig = current(x, y, c);
        Halide::Expr valBlur = blurred_sharp(x, y, c);
        Halide::Expr diff = valOrig - valBlur;
        f_sharpen(x, y, c) = valOrig + diff * sharpen_amount;
        current = f_sharpen;

        // Clarity (Gaussian)
        Halide::Func blurred_clarity =
            GaussianFilter::createHalideGraph(current, 2.0f, width, height);

        Halide::Func f_clarity;
        Halide::Expr valOrig2 = current(x, y, c);
        Halide::Expr valBlur2 = blurred_clarity(x, y, c);
        Halide::Expr diff2 = valOrig2 - valBlur2;
        f_clarity(x, y, c) = valOrig2 + diff2 * clarity_amount;
        current = f_clarity;

        // Final Clamp & Cast
        output(x, y, c) =
            Halide::cast<uint16_t>(Halide::clamp(current(x, y, c), 0.0f, 1.0f) * 65535.0f);

        // --- Constraints for Planar Layout (CRITICAL) ---
        // Both Input and Output MUST be Planar because ImageController creates and passes
        // Halide::Runtime::Buffer with standard planar constructor (stride(0)=1, stride(2)=w*h).
        // If we don't constrain this, Autoscheduler might assume Interleaved, reading valid planar
        // data as garbage.

        // Input Constraints
        input.dim(0).set_stride(1);
        input.dim(2).set_stride(input.dim(0).extent() * input.dim(1).extent());

        // Output Constraints
        output.dim(0).set_stride(1);
        output.dim(2).set_stride(input.dim(0).extent() * input.dim(1).extent());

        // Estimates (Help Auto-Scheduler just in case, though we used manual schedule above)
        output.dim(0).set_estimate(0, 6000);
        output.dim(1).set_estimate(0, 4000);
        output.dim(2).set_estimate(0, 3);
    }
};

HALIDE_REGISTER_GENERATOR(PhotoAdjustmentGenerator, photo_adjustment)

int main(int argc, char** argv) {
    return Halide::Internal::generate_filter_main(argc, argv);
}
