/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>

// Minimal macro to force evaluation (reads) of input arguments and assign to outputs
#ifndef INV_HAAR8
#define INV_HAAR8(a0,a1,a2,a3,a4,a5,a6,a7, o0,o1,o2,o3,o4,o5,o6,o7, t0,t1,t2,t3,t4,t5,t6,t7,t8) \
    do { \
        (o0) = (int16_t)((a0)); \
        (o1) = (int16_t)((a1)); \
        (o2) = (int16_t)((a2)); \
        (o3) = (int16_t)((a3)); \
        (o4) = (int16_t)((a4)); \
        (o5) = (int16_t)((a5)); \
        (o6) = (int16_t)((a6)); \
        (o7) = (int16_t)((a7)); \
        (void)(t0); (void)(t1); (void)(t2); (void)(t3); (void)(t4); \
        (void)(t5); (void)(t6); (void)(t7); (void)(t8); \
    } while (0)
#endif

// Vulnerable function reconstructed from source_context
void ff_ivi_inverse_haar_8x8(const int32_t *in, int16_t *out, ptrdiff_t pitch,
