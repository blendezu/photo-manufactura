#ifndef HALIDE_COLOR_SPACE_H
#define HALIDE_COLOR_SPACE_H

#include <vector>

#include "Halide.h"

class HalideColorSpace {
   public:
    static inline std::vector<Halide::Expr> BGR2HSL(Halide::Expr b, Halide::Expr g,
                                                    Halide::Expr r) {
        Halide::Expr cMax = Halide::max(r, Halide::max(g, b));
        Halide::Expr cMin = Halide::min(r, Halide::min(g, b));
        Halide::Expr delta = cMax - cMin;

        Halide::Expr L = (cMax + cMin) * 0.5f;

        // saturation
        Halide::Expr S =
            Halide::select(delta < 0.00001f, 0.0f, delta / (1.0f - Halide::abs(2.0f * L - 1.0f)));

        // branchless mit select from Halide
        Halide::Expr segment =
            Halide::select(cMax == r, g - b, Halide::select(cMax == g, b - r, r - g));

        Halide::Expr offset =
            Halide::select(cMax == r, 0.0f, Halide::select(cMax == g, 2.0f, 4.0f));

        Halide::Expr H = Halide::select(delta < 0.00001f, 0.0f, (segment / delta + offset));

        H = H * 60.0f;

        // if H < 0, + 360
        H = Halide::select(H < 0.0f, H + 360.0f, H);
        return {H, S, L};
    }

    static inline std::vector<Halide::Expr> HSL2BGR(Halide::Expr H, Halide::Expr S,
                                                    Halide::Expr L) {
        // 1. Chroma
        Halide::Expr C = (1.0f - Halide::abs(2.0f * L - 1.0f)) * S;

        Halide::Expr H_prime = H / 60.0f;

        // Modulo 2
        Halide::Expr mod2 = H_prime - 2.0f * Halide::floor(H_prime * 0.5f);
        Halide::Expr X = C * (1.0f - Halide::abs(mod2 - 1.0f));

        Halide::Expr m = L - C * 0.5f;

        // 2. sector logic
        // clang-format off

    Halide::Expr r_temp = Halide::select(H_prime < 1.0f, C,
                          Halide::select(H_prime < 2.0f, X,
                          Halide::select(H_prime < 3.0f, 0.0f,
                          Halide::select(H_prime < 4.0f, 0.0f,
                          Halide::select(H_prime < 5.0f, X, C)))));

    Halide::Expr g_temp = Halide::select(H_prime < 1.0f, X,
                          Halide::select(H_prime < 2.0f, C,
                          Halide::select(H_prime < 3.0f, C,
                          Halide::select(H_prime < 4.0f, X,
                          Halide::select(H_prime < 5.0f, 0.0f, 0.0f)))));

    Halide::Expr b_temp = Halide::select(H_prime < 1.0f, 0.0f,
                          Halide::select(H_prime < 2.0f, 0.0f,
                          Halide::select(H_prime < 3.0f, X,
                          Halide::select(H_prime < 4.0f, C,
                          Halide::select(H_prime < 5.0f, C, X)))));
        // clang-format on

        return {b_temp + m, g_temp + m, r_temp + m};
    }
};

#endif