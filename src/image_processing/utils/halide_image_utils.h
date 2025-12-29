#ifndef HALIDE_IMAGE_UTILS_H
#define HALIDE_IMAGE_UTILS_H

#include "Halide.h"

class HalideImageUtils {
   public:
    // Erzeugt den mathematischen Ausdruck für die Helligkeitsgewichtung
    static Halide::Expr calculateBrightWeight(Halide::Expr currVal, Halide::Expr underVal,
                                              Halide::Expr upperVal);

    // Erzeugt den mathematischen Ausdruck für die Dunkelgewichtung
    static Halide::Expr calculateDarkWeight(Halide::Expr currVal, Halide::Expr underVal,
                                            Halide::Expr upperVal);

    // Kubische Gewichtung (für Kurven etc.)
    static Halide::Expr calculateCubicWeight(Halide::Expr t);

    // --- Full Operations (ersetzt setSaturation, setVintage, blend) ---

    // Ändert die Sättigung
    static Halide::Func setSaturation(Halide::Func srcImg, Halide::Expr newSaturation,
                                      Halide::Expr maxVal);  // maxVal z.B. 255.0f

    // Wendet den Vintage Warm Filter an
    static Halide::Func applyWarmth(Halide::Func input, Halide::Expr maxVal);

    // Blendet Scratch/Kratzer über das Bild
    // scratchInput muss bereits passend skaliert sein (wir gehen von gleichen Koordinaten aus)
    static Halide::Func blendScratch(Halide::Func srcInput, Halide::Func scratchInput,
                                     Halide::Expr maxVal, bool isColor);
};

#endif