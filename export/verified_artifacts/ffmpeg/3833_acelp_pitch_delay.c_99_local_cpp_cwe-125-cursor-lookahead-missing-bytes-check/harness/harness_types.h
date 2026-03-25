/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Neutralized entry function: direct pass-through to vulnerable function
int entry_func(float fixed_gain_factor, float fixed_mean_energy,
               float *prediction_error, float energy_mean,
               const float *pred_table);

float ff_amr_set_fixed_gain(float fixed_gain_factor, float fixed_mean_energy,
                            float *prediction_error, float energy_mean,
