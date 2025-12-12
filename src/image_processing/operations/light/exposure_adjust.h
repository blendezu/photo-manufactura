#ifndef EXPOSURE_ADJUST_H
#define EXPOSURE_ADJUST_H

#include "../../core/operation_base.h"

/**
 * @brief Adjusts the exposure of the image.
 *
 * Exposure adjustment simulates changing the exposure time or aperture of the camera.
 * Typically implemented as a multiplication: pixel = pixel * 2^EV.
 */
class AdjustExposure : public HalideOperation {
   public:
    /**
     * @brief Construct a new Adjust Exposure object
     *
     * @param exposure The exposure value in EV (stops).
     *                 0.0 is neutral.
     *                 +1.0 doubles the brightness.
     *                 -1.0 halves the brightness.
     */
    explicit AdjustExposure(float exposure);

    cv::Mat apply(const cv::Mat& image) override;

    std::string getName() const override;

    // Halide Interface
    Halide::Func buildGraph(Halide::Func input, Halide::Var x, Halide::Var y,
                            Halide::Var c) override;
    void prepareParameters(const cv::Mat& srcImg) override;

    // Parameter for Halide
    Halide::Param<float> p_factor;

   private:
    float m_exposure;

    // Templated CPU implementation
    template <typename T>
    cv::Mat exposureTemplate(const cv::Mat& img, float factor);
};

#endif  // EXPOSURE_ADJUST_H
