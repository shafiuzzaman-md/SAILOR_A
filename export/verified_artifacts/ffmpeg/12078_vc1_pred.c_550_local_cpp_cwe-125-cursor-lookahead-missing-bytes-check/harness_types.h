/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for vc1_pred.c OOB lookahead at line ~550 */
#include <stdint.h>
#include <stddef.h>

#ifndef XML_HIDDEN
#define XML_HIDDEN  /* empty */
#endif

struct _xmlDict { int seed; int size; void *table; void *subdict; int limit; };

/* Minimal type shells to support the harness */
typedef struct MpegEncContext {
    int *block_index;  /* array of block indices */
    int mb_x, mb_stride, mb_width, first_slice_line; /* unused here */
} MpegEncContext;

typedef struct VC1Context {
    MpegEncContext s; /* embedded */
    /* other fields omitted */
} VC1Context;

/* Vulnerable function (neutralized): keep signature-compatible and include the exact vulnerable statement */
void ff_vc1_pred_mv(VC1Context *v, int n, int dmv_x, int dmv_y,
                    int mv1, int r_x, int r_y, uint8_t* is_intra,
