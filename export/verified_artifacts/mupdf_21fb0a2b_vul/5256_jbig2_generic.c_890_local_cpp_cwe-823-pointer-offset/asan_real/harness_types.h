/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Minimal type definitions to satisfy signatures
typedef struct { int dummy; } Jbig2Ctx;
typedef struct { int number; } Jbig2Segment;
typedef struct { int dummy; } Jbig2ArithState;
typedef struct { int dummy; } Jbig2ArithCx;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint8_t *data;
} Jbig2Image;

typedef struct {
    int MMR;
    int TPGDON;
    int GBTEMPLATE;
    int USESKIP;
    int8_t gbat[8];
} Jbig2GenericRegionParams;

// Vulnerable function: keep exact vulnerable statement and add sink assertion
