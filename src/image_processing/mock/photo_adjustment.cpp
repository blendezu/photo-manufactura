#include "photo_adjustment.h"

#include <iostream>

#include "HalideRuntime.h"

extern "C" {

int photo_adjustment(halide_buffer_t* input, float exposure_factor, float contrast_factor,
                     float brightness, float highlight_factor, float highlight_under,
                     float highlight_upper, float shadow_factor, float shadow_under,
                     float shadow_upper, float white_factor, float white_under, float white_upper,
                     float black_factor, float black_lower, float black_upper,
                     float saturation_factor, float vibrance, float tint_magenta, float wb_factor_r,
                     float wb_factor_b, float sharpen_amount, float clarity_amount,
                     float denoise_sigma_spatial, float denoise_sigma_range, float denoise_blend,
                     halide_buffer_t* output) {
    // Mock implementation: Copy input to output
    // This allows the app to run and verify the pipeline integration logic.
    // In real AOT, this would run the Halide code.

    // For now, assuming interleaved 8-bit.
    if (!input || !output)
        return -1;
    if (input->host == nullptr || output->host == nullptr)
        return -2;

    // Check sizes
    // Just simple copy
    size_t size = input->dim[0].extent * input->dim[1].extent * input->dim[2].extent;
    uint8_t* src = input->host;
    uint8_t* dst = output->host;

    // std::copy
    for (size_t i = 0; i < size; ++i) {
        dst[i] = src[i];
    }

    // std::cout << "AOT Pipeline Executed (Mock)" << std::endl;
    return 0;
}
}
