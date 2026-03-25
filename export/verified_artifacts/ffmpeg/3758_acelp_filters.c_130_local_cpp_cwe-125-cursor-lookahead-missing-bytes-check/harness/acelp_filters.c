#include <klee/klee.h>
#include <stddef.h>

// Vulnerable function (neutralized, but keeping the exact vulnerable statement)
void ff_acelp_apply_order_2_transfer_function(float *out, const float *in,
                                              const float zero_coeffs[2],
                                              const float pole_coeffs[2],
                                              float gain, float mem[2], int n)
{
    int i;
    float tmp;

    for (i = 0; i < n; i++) {
        tmp = gain * in[i] - pole_coeffs[0] * mem[0] - pole_coeffs[1] * mem[1];
        klee_assert(0 && "SAILOR_SINK_REACHED");
        out[i] =       tmp + zero_coeffs[0] * mem[0] + zero_coeffs[1] * mem[1];

        mem[1] = mem[0];
        mem[0] = tmp;
    }
}

// Entry must be a direct pass-through with no guards
int entry_func(float *out, const float *in,
               const float zero_coeffs[2],
               const float pole_coeffs[2],
               float gain, float mem[2], int n) {
    ff_acelp_apply_order_2_transfer_function(out, in, zero_coeffs, pole_coeffs, gain, mem, n);
    return 0;
}
