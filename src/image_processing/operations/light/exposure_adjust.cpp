#include "exposure_adjust.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "../../core/halide_build_graph.h"

AdjustExposure::AdjustExposure(float exposure)
    : m_exposure(exposure) {  // HalideOperation has no parameterized constructor
    // Initialize Halide parameter
    // Default factor is 1.0 (2^0)
    p_factor.set(std::pow(2.0f, m_exposure));
}

std::string AdjustExposure::getName() const {
    return "Exposure";
}

cv::Mat AdjustExposure::apply(const cv::Mat& image) {
    // Note: image is const here, but we usually return a NEW image in apply() or modify in place?
    // ImageOperation::apply returns cv::Mat.
    // Wait, ImageOperation::apply returns cv::Mat.
    // My previous implementation returned void?
    // Let's check ImageOperation::apply signature again.
    // virtual cv::Mat apply(const cv::Mat& srcImg) = 0;
    // So it MUST return cv::Mat.

    // My previous implementation was `void apply(...)`. That was wrong too.
    // The previous implementation tried to modify in place?
    // But input is const. So I must allocate new and return.

    float factor = std::pow(2.0f, m_exposure);
    // Cast const away? No, just use as source.
    cv::Mat src = image;

    if (src.depth() == CV_8U) {
        if (src.channels() == 3) {
            return exposureTemplate<cv::Vec3b>(src, factor);
        } else {
            return exposureTemplate<uint8_t>(src, factor);
        }
    } else if (src.depth() == CV_16U) {
        if (src.channels() == 3) {
            return exposureTemplate<cv::Vec3w>(src, factor);
        } else {
            return exposureTemplate<uint16_t>(src, factor);
        }
    } else if (src.depth() == CV_32F) {
        if (src.channels() == 3) {
            return exposureTemplate<cv::Vec3f>(src, factor);
        } else {
            return exposureTemplate<float>(src, factor);
        }
    } else {
        std::cerr << "[AdjustExposure] Unsupported image type for CPU implementation.\n";
        return src.clone();
    }
}

template <typename T>
cv::Mat AdjustExposure::exposureTemplate(const cv::Mat& img, float factor) {
    // Create dst
    cv::Mat dst = img.clone();  // Clone to get same size/type

    // Determine max value based on type for clamping
    double maxVal = 255.0;
    if constexpr (std::is_same_v<T, cv::Vec3b> || std::is_same_v<T, uint8_t>) {
        maxVal = 255.0;
    } else if constexpr (std::is_same_v<T, cv::Vec3w> || std::is_same_v<T, uint16_t>) {
        maxVal = 65535.0;
    } else {
        maxVal = 1.0;
    }

    dst.forEach<T>([&](T& pixel, const int* position) -> void {
        (void)position;

        if constexpr (std::is_compound_v<T>) {  // Color (Vec3)
            for (int c = 0; c < 3; c++) {
                float val = static_cast<float>(pixel[c]) * factor;
                pixel[c] =
                    static_cast<typename T::value_type>(std::clamp(val, 0.0f, (float)maxVal));
            }
        } else {  // Grayscale (Scalar)
            float val = static_cast<float>(pixel) * factor;
            pixel = static_cast<T>(std::clamp(val, 0.0f, (float)maxVal));
        }
    });

    return dst;
}

// --- Halide Implementation ---

void AdjustExposure::prepareParameters(const cv::Mat& srcImg) {
    (void)srcImg;
    // Recalculate factor in case m_exposure changed
    float factor = std::pow(2.0f, m_exposure);
    p_factor.set(factor);
}

Halide::Func AdjustExposure::buildGraph(Halide::Func input, Halide::Var x, Halide::Var y,
                                        Halide::Var c) {
    (void)x;
    (void)y;
    (void)c;

    // Formula: output = input * factor
    // Shared logic generates a Func, we can use it or just call the logic inline?
    // halide_build_graph.h apply_exposure returns a Func.
    // We can just call it assignment style:
    Halide::Func output = HalideBuildGraph::apply_exposure(input, p_factor);
    return output;
}
