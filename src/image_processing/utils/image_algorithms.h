#pragma once
#include <Halide.h>

#include "halide_image_utils.h"

namespace ImageAlgorithms {
using namespace Halide;

// Apply Contrast: (val - 0.5) * factor + 0.5
// Expects normalized values (0.0 - 1.0) usually Luminance.
inline Expr apply_contrast(Expr val, Expr factor) {
    return (val - 0.5f) * factor + 0.5f;
}

// Apply Saturation: val * factor (clamped)
// Expects normalized Saturation (0.0 - 1.0).
inline Expr apply_saturation(Expr val, Expr factor) {
    return clamp(val * factor, 0.0f, 1.0f);
}

// Apply Brightness: val * factor (clamped)
// Multiplicative brightness (Exposure-like)
inline Expr apply_brightness(Expr val, Expr factor) {
    return clamp(val * factor, 0.0f, 1.0f);
}

// Apply Vibrance: Sat + Weight * Factor
// Boosts lower saturation pixels more than high saturation ones.
inline Expr apply_vibrance(Expr sat, Expr factor, Expr lowThresh, Expr highThresh) {
    Expr weight = HalideImageUtils::calculateDarkWeight(sat, lowThresh, highThresh);
    return clamp(sat + weight * factor, 0.0f, 1.0f);
}
}  // namespace ImageAlgorithms
