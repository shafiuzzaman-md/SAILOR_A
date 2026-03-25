// NO_HARNESS_TYPES
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

// entry_func is provided by the harness
int entry_func(float fixed_gain_factor, float fixed_mean_energy,
               float *prediction_error, float energy_mean,
               const float *pred_table);

int main() {
    // Allocate an undersized prediction_error buffer to trigger OOB read
    float *prediction_error = (float *)malloc(3 * sizeof(float));
    if (!prediction_error) return 0;
    klee_make_symbolic(prediction_error, 3 * sizeof(float), "prediction_error");

    // Other parameters
    float fixed_gain_factor = 1.0f;
    float fixed_mean_energy = 1.0f;
    float energy_mean = 0.0f;

    // pred_table: small valid buffer
    float *pred_table = (float *)malloc(4 * sizeof(float));
    if (!pred_table) return 0;
    klee_make_symbolic(pred_table, 4 * sizeof(float), "pred_table");

    entry_func(fixed_gain_factor, fixed_mean_energy,
               prediction_error, energy_mean, pred_table);
    return 0;
}
