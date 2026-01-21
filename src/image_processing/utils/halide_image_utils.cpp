#include "halide_image_utils.h"

#include <Halide.h>

#include "halide_color_space.h"

Halide::Expr HalideImageUtils::calculateBrightWeight(Halide::Expr currVal, Halide::Expr underVal,
                                                     Halide::Expr upperVal) {
    // avoid 0 division direct in graph
    Halide::Expr range = upperVal - underVal;
    Halide::Expr invRange = Halide::select(range < 1e-6f, 0.0f, 1.0f / range);

    Halide::Expr t = (upperVal - currVal) * invRange;
    Halide::Expr weightCurve = 1.0f - (t * t);

    return Halide::select(currVal <= underVal, 0.0f,
                          Halide::select(currVal >= upperVal, 1.0f, weightCurve));
}

Halide::Expr HalideImageUtils::calculateDarkWeight(Halide::Expr currVal, Halide::Expr underVal,
                                                   Halide::Expr upperVal) {
    Halide::Expr range = upperVal - underVal;
    Halide::Expr invRange = Halide::select(range < 1e-6f, 0.0f, 1.0f / range);

    Halide::Expr t = (currVal - underVal) * invRange;
    Halide::Expr weightCurve = 1.0f - (t * t);

    return Halide::select(currVal >= upperVal, 0.0f,
                          Halide::select(currVal >= underVal, weightCurve, 1.0f));
}

Halide::Expr HalideImageUtils::calculateCubicWeight(Halide::Expr t_in) {
    Halide::Expr t = Halide::abs(t_in);

    Halide::Expr case1 = 1.5f * (t * t * t) - 2.5f * (t * t) + 1.0f;

    Halide::Expr case2 = -0.5f * (t * t * t) + 2.5f * (t * t) - 4.0f * t + 2.0f;

    return Halide::select(t < 1.0f, case1, Halide::select(t < 2.0f, case2, 0.0f));
}

Halide::Func HalideImageUtils::setSaturation(Halide::Func srcImg, Halide::Expr newSaturation,
                                             Halide::Expr maxVal) {
    Halide::Var x("x"), y("y"), c("c");

    // normalization
    Halide::Expr invMaxVal = 1.0f / maxVal;  // to avoid Division
    Halide::Expr b = srcImg(x, y, 0) * invMaxVal;
    Halide::Expr g = srcImg(x, y, 1) * invMaxVal;
    Halide::Expr r = srcImg(x, y, 2) * invMaxVal;

    // cvt to HSL
    std::vector<Halide::Expr> hsl = HalideColorSpace::BGR2HSL(b, g, r);
    Halide::Expr H = hsl[0];
    Halide::Expr L = hsl[2];

    // new saturation value
    Halide::Expr newS = newSaturation;

    // cvt back to BGR
    std::vector<Halide::Expr> bgr = HalideColorSpace::HSL2BGR(H, newS, L);

    // output
    Halide::Func dstImg("saturation_image");
    Halide::Expr dstVal = Halide::select(c == 0, bgr[0], Halide::select(c == 1, bgr[1], bgr[2]));

    // scale back to 8 or 16 bit
    dstImg(x, y, c) = Halide::clamp(dstVal * maxVal, 0.0f, maxVal);

    return dstImg;
}

Halide::Func HalideImageUtils::applyWarmth(Halide::Func srcImg, Halide::Expr maxVal) {
    Halide::Var x("x"), y("y"), c("c");

    Halide::Expr val = srcImg(x, y, c);

    // define factor
    Halide::Expr factorB = 0.96875f;
    Halide::Expr factorG = 1.04980f;
    Halide::Expr factorR = 1.09960f;

    Halide::Expr factor = Halide::select(c == 0, factorB, Halide::select(c == 1, factorG, factorR));

    Halide::Func dstImg("warm_image");

    dstImg(x, y, c) = Halide::clamp(val * factor, 0.0f, maxVal);

    return dstImg;
}

Halide::Func HalideImageUtils::blendScratch(Halide::Func srcImg, Halide::Func scratchImg,
                                            Halide::Expr maxVal, bool isColor) {
    Halide::Var x("x"), y("y"), c("c");

    Halide::Expr isScratch;           // to check if the pixel belongs to scratch
    Halide::Expr threshold = 200.0f;  //

    if (isColor) {
        Halide::Expr sB = scratchImg(x, y, 0);
        Halide::Expr sG = scratchImg(x, y, 1);
        Halide::Expr sR = scratchImg(x, y, 2);

        isScratch = (sB > threshold) && (sG > threshold) && (sR > threshold);
    } else {
        Halide::Expr sGray = scratchImg(x, y, 0);
        isScratch = sGray > threshold;
    }

    Halide::Expr srcPixel = srcImg(x, y, c);
    Halide::Expr white = maxVal;

    Halide::Func dstImg("blended_image");
    dstImg(x, y, c) = Halide::select(isScratch, white, srcPixel);

    return dstImg;
}
