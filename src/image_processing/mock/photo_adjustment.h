#ifndef HALIDE_photo_adjustment_H
#define HALIDE_photo_adjustment_H

struct halide_buffer_t;

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration of AOT function
// matched to AOT generated header
int photo_adjustment(struct halide_buffer_t* _input_buffer, float _exposure_factor,
                     float _contrast_factor, float _brightness, float _highlight_factor,
                     float _highlight_under, float _highlight_upper, float _shadow_factor,
                     float _shadow_under, float _shadow_upper, float _white_factor,
                     float _white_under, float _white_upper, float _black_factor,
                     float _black_lower, float _black_upper, float _saturation_factor,
                     float _vibrance, float _tint_magenta_factor, float _wb_factor_r,
                     float _wb_factor_b, float _sharpen_amount, float _clarity_amount,
                     float _denoise_sigma_spatial, float _denoise_sigma_range, float _denoise_blend,
                     struct halide_buffer_t* _output_buffer);

#ifdef __cplusplus
}
#endif

#endif
