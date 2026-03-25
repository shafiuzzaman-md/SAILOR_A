/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness spine for jbig2_generic.c vulnerability */
#include <stdint.h>
#include <stdlib.h>

/* Minimal type defs to satisfy signatures */
typedef struct Jbig2Ctx { int dummy; } Jbig2Ctx;
typedef struct Jbig2Segment { int number; } Jbig2Segment;

typedef struct Jbig2Image {
    int width;
    int height;
    int stride;
    uint8_t *data;
} Jbig2Image;

typedef struct Jbig2ArithState { int dummy; } Jbig2ArithState;
typedef uint16_t Jbig2ArithCx; /* context array element type */

typedef struct Jbig2GenericRegionParams {
    int MMR;
    int TPGDON;
    int GBTEMPLATE;
    int USESKIP;
    int GBW;
    int GBH;
    int8_t gbat[8];
    Jbig2Image *SKIP; /* only used if USESKIP */
} Jbig2GenericRegionParams;

/* Forward decl of vulnerable function */
static int jbig2_decode_generic_template1(Jbig2Ctx *ctx,
                               Jbig2Segment *segment,
                               const Jbig2GenericRegionParams *params, Jbig2ArithState *as, Jbig2Image *image, Jbig2ArithCx *GB_stats);

/* ENTRY: pass-through to the vulnerable function */
int jbig2_decode_generic_region(Jbig2Ctx *ctx,
