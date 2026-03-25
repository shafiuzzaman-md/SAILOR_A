/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <string.h>

// Minimal type needed by the vulnerable function
typedef struct AVTWOFISH {
    uint32_t K[40];
    uint32_t S[4];
    int ksize;
    uint32_t MDS1[256];
    uint32_t MDS2[256];
    uint32_t MDS3[256];
    uint32_t MDS4[256];
} AVTWOFISH;

// NEUTRALIZED vulnerable function: keep signature and sink path only
