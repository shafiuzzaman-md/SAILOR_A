/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <string.h>

// Minimal local reproduction of the needed context type
// Keep only fields used on the direct path
typedef struct MpegEncContext {
    int b8_stride;
    int block_index[6];
    // ac_val: per-plane arrays of 16-int16_t blocks
    int16_t (*ac_val[3])[16];
} MpegEncContext;

// Vulnerable function (neutralized to the minimal path)
