/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef FFMIN
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#endif

// Minimal struct to support bprint operations
typedef struct AVBPrint {
    char *str;
    unsigned len;
    unsigned size;
} AVBPrint;

// Minimal prototypes used by av_bprint_chars

// Vulnerable function (neutralized, exact vulnerable statement preserved)
