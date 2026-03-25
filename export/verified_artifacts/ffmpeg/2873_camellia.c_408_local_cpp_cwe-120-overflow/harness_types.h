/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <string.h>

// Minimal type needed from project
typedef struct AVCAMELLIA {
    uint64_t Kw[4];
    uint64_t Ke[6];
    uint64_t K[24];
    int key_bits;
} AVCAMELLIA;

// Neutralized vulnerable function: keep signature and the exact vulnerable statement
