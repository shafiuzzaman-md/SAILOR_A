/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Forward declarations for project types (opaque here)
typedef struct Jbig2Ctx Jbig2Ctx;
typedef struct Jbig2Segment Jbig2Segment;
typedef struct Jbig2GenericRegionParams Jbig2GenericRegionParams;
typedef struct Jbig2ArithState Jbig2ArithState;
typedef struct Jbig2Image Jbig2Image;
typedef struct Jbig2ArithCx Jbig2ArithCx;

// Global used to feed the vulnerable pointer
extern uint8_t *g_line2;

// Vulnerable function (neutralized to the sink)
static int jbig2_decode_generic_template2(Jbig2Ctx *ctx,
                                Jbig2Segment *segment,
                                const Jbig2GenericRegionParams *params,
                                Jbig2ArithState *as,
                                Jbig2Image *image,
                                Jbig2ArithCx *GB_stats)
{
    // Minimal locals to host the exact vulnerable statement
    uint8_t *line2 = g_line2;
    uint32_t line_m2;

    // === VULNERABLE STATEMENT (verbatim from source_context) ===
    line_m2 = line2 ? line2[0] << 4 : 0;
    // Universal sink assertion placed AFTER the vulnerable statement

    (void)ctx; (void)segment; (void)params; (void)as; (void)image; (void)GB_stats; (void)line_m2;
    return 0;
}

// Entry function (MANDATORY: direct pass-through, no guards)
int jbig2_decode_generic_region(Jbig2Ctx *ctx,
                            Jbig2Segment *segment, const Jbig2GenericRegionParams *params,
