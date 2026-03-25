/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>

// Vulnerable function (neutralized, but keeping the exact vulnerable statement)
void ff_acelp_apply_order_2_transfer_function(float *out, const float *in,
                                              const float zero_coeffs[2],
                                              const float pole_coeffs[2],
                                              float gain, float mem[2], int n)
{
    int i;
    float tmp;
