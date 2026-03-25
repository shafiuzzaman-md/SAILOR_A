#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <klee/klee.h>

// Neutralized entry function: direct pass-through to vulnerable function
int entry_func(float fixed_gain_factor, float fixed_mean_energy,
               float *prediction_error, float energy_mean,
               const float *pred_table);

float ff_amr_set_fixed_gain(float fixed_gain_factor, float fixed_mean_energy,
                            float *prediction_error, float energy_mean,
                            const float *pred_table)
{
    // Neutralized prelude: avoid external heavy calls; keep signature only
    float val = fixed_gain_factor;  // placeholder

    // update quantified prediction error energy history
    // VULNERABLE STATEMENT — must be verbatim from source
    memmove(&prediction_error[0], &prediction_error[1],
            3 * sizeof(prediction_error[0]));
    // Universal sink assertion: fires only if the above didn't already crash
    klee_assert(0 && "SAILOR_SINK_REACHED");

    // Skip the original log10f assignment to avoid deps; return placeholder
    return val;
}

int entry_func(float fixed_gain_factor, float fixed_mean_energy,
               float *prediction_error, float energy_mean,
               const float *pred_table) {
    ff_amr_set_fixed_gain(fixed_gain_factor, fixed_mean_energy,
                          prediction_error, energy_mean, pred_table);
    return 0;
}
