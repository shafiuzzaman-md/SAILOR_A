/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local typedefs/macros to compile the slice
#ifndef MV_FWD_B2
#define MV_FWD_B2 1
#endif
#ifndef MV_BWD_B2
#define MV_BWD_B2 5
#endif

// Minimal cavs_vector type
typedef struct cavs_vector {
    int16_t x;
    int16_t y;
    int16_t ref;
} cavs_vector;

// Minimal AVSContext carrying only fields we use here
typedef struct AVSContext {
    int mbx;
    int mb_width;
    cavs_vector mv[64];
    cavs_vector *top_mv[2];
} AVSContext;

// Vulnerable function (neutralized) — keep only the for-loop with the exact vulnerable statement
