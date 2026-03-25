/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c — neutralized slice for mvs.c target */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef XML_HIDDEN
#define XML_HIDDEN /* empty */
#endif

/* Locally define unknown structs/macros if needed */
struct _xmlDict { int seed; int size; void *table; void *subdict; int limit; };

/* Minimal type sketches extracted from observed source around target */
typedef struct Mv { int16_t x, y; } Mv;

typedef struct MvField {
    uint8_t pred_flag;
    Mv mv[2];
    int8_t ref_idx[2];
} MvField;

#ifndef PF_L0
#define PF_L0 1
#endif
#ifndef PF_L1
#define PF_L1 2
#endif
#ifndef PF_BI
#define PF_BI 3
#endif

#ifndef HEVC_SLICE_B
#define HEVC_SLICE_B 1
#endif

#ifndef MRG_MAX_NUM_CANDS
#define MRG_MAX_NUM_CANDS 5
#endif

