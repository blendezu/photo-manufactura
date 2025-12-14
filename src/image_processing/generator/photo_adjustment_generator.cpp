#include "../core/halide_build_graph.h"
#include "../utils/halide_color_space.h"
#include "Halide.h"

class PhotoAdjustmentGenerator : public Halide::Generator<PhotoAdjustmentGenerator> {
   public:
    // --- 1. Define Halide Variables ---

    // --- 1.1 Source Image ---
    Halide::GeneratorInput<Halide::Buffer<uint16_t>> srcImg{"srcImg", 3};  // HWC 16-bit image

    // --- 1.2 Light Parameters ---
    Halide::GeneratorInput<float> exposureFactor{"exposureFactor"};  // pow(2, exposure)
    Halide::GeneratorInput<float> contrastFactor{"contrastFactor"};  // 1.0 + contrast / 50.0
    Halide::GeneratorInput<float> brightnessFactor{"brightnessFactor"};

    // --- 1.3 Tone Mapping Parameters ---
    Halide::GeneratorInput<float> highlightFactor{"highlightFactor"};
    Halide::GeneratorInput<float> highlightUnder{"highlightUnder"};
    Halide::GeneratorInput<float> highlightUpper{"highlightUpper"};

    Halide::GeneratorInput<float> shadowFactor{"shadowFactor"};
    Halide::GeneratorInput<float> shadowUnder{"shadowUnder"};
    Halide::GeneratorInput<float> shadowUpper{"shadowUpper"};

    Halide::GeneratorInput<float> whiteFactor{"whiteFactor"};
    Halide::GeneratorInput<float> whiteUnder{"whiteUnder"};
    Halide::GeneratorInput<float> whiteUpper{"whiteUpper"};

    Halide::GeneratorInput<float> blackFactor{"blackFactor"};
    Halide::GeneratorInput<float> blackLower{"blackLower"};
    Halide::GeneratorInput<float> blackUpper{"blackUpper"};

    // --- 1.4 Color Parameters ---
    Halide::GeneratorInput<float> saturationFactor{"saturationFactor"};
    Halide::GeneratorInput<float> vibranceFactor{"vibranceFactor"};

    // --- 1.5 Temp/Tint ---
    Halide::GeneratorInput<float> tintMagentaFactor{"tintMagentaFactor"};

    // --- 1.6 White Balance ---
    Halide::GeneratorInput<float> wbFactorR{"wbFactorR"};
    Halide::GeneratorInput<float> wbFactorB{"wbFactorB"};

    // --- 1.7 Detail Parameters ---
    // Sharpen
    Halide::GeneratorInput<float> sharpenAmount{"sharpenAmount"};

    // Clarity
    Halide::GeneratorInput<float> clarityAmount{"clarityAmount"};

    // --- 1.8 Output ---
    Halide::GeneratorOutput<Halide::Buffer<uint16_t>> dstImg{"dstImg", 3};

    // --- 2. Define Halide Functions ---

    // This method defines the image processing pipeline and sets estimates for Halide's
    // auto-scheduler. It is called by Halide's auto-scheduler to generate the best possible
    // schedule for the given input image size and parameters.
    void generate() {
        // --- 2.1. Estimates for Auto-Scheduler ---
        // Estimates are crucial for Halide's auto-scheduler to effectively explore the vast search
        // space of possible schedules. By providing typical ranges and values for input dimensions
        // and parameters, we guide the auto-scheduler towards optimal performance for common use
        // cases. Without estimates, the auto-scheduler might struggle to find an efficient schedule
        // or take significantly longer to do so, as it would have to make assumptions about the
        // input characteristics.

        // 2.1.1 Input Buffer
        srcImg.dim(0).set_estimate(0, 6000);
        srcImg.dim(1).set_estimate(0, 4000);
        srcImg.dim(2).set_estimate(0, 3);

        // 2.1.2 Light Params
        exposureFactor.set_estimate(1.0f);
        contrastFactor.set_estimate(1.0f);
        brightnessFactor.set_estimate(1.0f);

        highlightFactor.set_estimate(0.0f);
        highlightUnder.set_estimate(0.5f);
        highlightUpper.set_estimate(1.0f);

        shadowFactor.set_estimate(0.0f);
        shadowUnder.set_estimate(0.0f);
        shadowUpper.set_estimate(0.5f);

        whiteFactor.set_estimate(0.0f);
        whiteUnder.set_estimate(0.8f);
        whiteUpper.set_estimate(1.0f);

        blackFactor.set_estimate(0.0f);
        blackLower.set_estimate(0.0f);
        blackUpper.set_estimate(0.2f);

        // 2.1.3 Color Params
        saturationFactor.set_estimate(1.0f);
        vibranceFactor.set_estimate(1.0f);
        tintMagentaFactor.set_estimate(1.0f);
        wbFactorR.set_estimate(1.0f);
        wbFactorB.set_estimate(1.0f);

        // 2.1.4 Detail Params
        sharpenAmount.set_estimate(0.0f);
        clarityAmount.set_estimate(0.0f);

        // --- 2.2 Define Halide Variables ---
        Halide::Var x("x"), y("y"), c("c");

        // --- 2.3 Safe Access to Input ---
        Halide::Func inFunc = Halide::BoundaryConditions::repeat_edge(srcImg);

        // --- 2.4 Cast Input to Float (0..1) ---
        Halide::Expr invMaxRange = 1.0f / 65535.0f;
        Halide::Expr val = Halide::cast<float>(inFunc(x, y, c)) * invMaxRange;

        // --- 2.5 Sequence of Operations ---
        // There are 3 blocks of Operations:
        // 1. BGR Block: Exposure -> White Balance -> Tint
        // 2. HSL Block: Brightness -> Tone Mapping (Highlight, Shadow, White, Black) -> Contrast ->
        // Saturation -> Vibrance
        // 3. Detail Block: Sharpen -> Clarity
        // In this way we can avoid Converting back and forth between BGR and HSL

        // 2.5.1 BGR Block
        Halide::Func bgrImg;
        bgrImg(x, y, c) = val;

        // 2.5.1.1 Exposure
        bgrImg = HalideBuildGraph::apply_exposure(bgrImg, exposureFactor);

        // 2.5.1.2 White Balance
        bgrImg = HalideBuildGraph::apply_white_balance(bgrImg, wbFactorR, wbFactorB);

        // 2.5.1.3 Tint (Magenta/Green) - Applied in BGR for correctness
        bgrImg = HalideBuildGraph::apply_tint(bgrImg, tintMagentaFactor);

        // --- 2.5.2 HSL Block ---

        // 2.5.2.1 BGR -> HSL Conversion
        Halide::Expr R = bgrImg(x, y, 2);
        Halide::Expr G = bgrImg(x, y, 1);
        Halide::Expr B = bgrImg(x, y, 0);
        std::vector<Halide::Expr> hslImg = HalideColorSpace::BGR2HSL(B, G, R);

        Halide::Expr H = hslImg[0];
        Halide::Expr S = hslImg[1];
        Halide::Expr L = hslImg[2];

        // 2.5.2.2 Brightness
        L = HalideBuildGraph::apply_brightness_L(L, brightnessFactor);

        // 2.5.2.3 Tone Mapping
        L = HalideBuildGraph::apply_highlight_L(L, highlightFactor, highlightUnder, highlightUpper);
        L = HalideBuildGraph::apply_shadow_L(L, shadowFactor, shadowUnder, shadowUpper);
        L = HalideBuildGraph::apply_white_L(L, whiteFactor, whiteUnder, whiteUpper);
        L = HalideBuildGraph::apply_black_L(L, blackFactor, blackLower, blackUpper);

        // 2.5.2.4 Contrast
        L = HalideBuildGraph::apply_contrast_L(L, contrastFactor);

        // 2.5.2.5 Saturation
        S = HalideBuildGraph::apply_saturation_S(S, saturationFactor);

        // 2.5.2.6 Vibrance
        S = HalideBuildGraph::apply_vibrance_S(S, vibranceFactor, 0.35f, 0.45f);

        // 2.5.2.7 HSL -> BGR Conversion
        std::vector<Halide::Expr> new_bgrImg = HalideColorSpace::HSL2BGR(H, S, L);

        Halide::Func currentImg;
        currentImg(x, y, c) = Halide::select(c == 0, new_bgrImg[0],
                                             Halide::select(c == 1, new_bgrImg[1], new_bgrImg[2]));

        // 2.5.3 Spatial Block (Post-Processing)

        // Get image dimensions for Sharpen & Clarity
        Halide::Expr width = srcImg.dim(0).extent();
        Halide::Expr height = srcImg.dim(1).extent();

        // 2.5.3.1 Sharpen
        currentImg = HalideBuildGraph::apply_sharpen(currentImg, sharpenAmount, width, height);

        // 2.5.3.2 Clarity
        currentImg = HalideBuildGraph::apply_clarity(currentImg, clarityAmount, width, height);

        // 2.5.3.3 Final Clamp, Cast and Assign to Destination Image
        dstImg(x, y, c) =
            Halide::cast<uint16_t>(Halide::clamp(currentImg(x, y, c), 0.0f, 1.0f) * 65535.0f);

        // --- 2.6 Constraints for Planar Layout (CRITICAL) ---
        // Both Input and Output MUST be Planar because ImageController creates and passes
        // Halide::Runtime::Buffer with standard planar constructor (stride(0)=1, stride(2)=w*h).
        // If we don't constrain this, Autoscheduler might assume Interleaved, reading valid planar
        // data as garbage.

        // 2.6.1 Input Constraints
        srcImg.dim(0).set_stride(1);
        srcImg.dim(2).set_stride(srcImg.dim(0).extent() * srcImg.dim(1).extent());

        // 2.6.2 Output Constraints
        dstImg.dim(0).set_stride(1);
        dstImg.dim(2).set_stride(srcImg.dim(0).extent() * srcImg.dim(1).extent());

        // 2.6.3 Estimates the Output Dimensions
        dstImg.dim(0).set_estimate(0, 6000);
        dstImg.dim(1).set_estimate(0, 4000);
        dstImg.dim(2).set_estimate(0, 3);
    }
};

HALIDE_REGISTER_GENERATOR(PhotoAdjustmentGenerator, photo_adjustment)

int main(int argc, char** argv) {
    return Halide::Internal::generate_filter_main(argc, argv);
}
