/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for jpeg_idct_1x1 */
#include <stdint.h>
#include <stdlib.h>

#ifndef RANGE_CENTER
#define RANGE_CENTER 128
#endif
#ifndef RANGE_MASK
#define RANGE_MASK 1023
#endif

/* Minimal JPEG types */
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE;
typedef int DCTELEM;
typedef int ISLOW_MULT_TYPE;
typedef int JCOEF;
typedef JCOEF* JCOEFPTR;
typedef JSAMPLE* JSAMPROW;
typedef JSAMPROW* JSAMPARRAY;

struct jpeg_decompress_struct { int dummy; };
typedef struct jpeg_decompress_struct* j_decompress_ptr;

/* Minimal component info carrying dct_table as required by entry precondition */
typedef struct jpeg_component_info {
    void *dct_table;
} jpeg_component_info;

/* Macros used by the function */
#ifndef ISHIFT_TEMPS
#define ISHIFT_TEMPS /* no temps needed in this harness */
#endif
#ifndef IRIGHT_SHIFT
#define IRIGHT_SHIFT(x,shft) ((x) >> (shft))
#endif
#ifndef CLAMP_DC
#define CLAMP_DC(x) /* no-op for harness */
#endif
#ifndef DEQUANTIZE
#define DEQUANTIZE(coef,quant) ((DCTELEM)((coef) * (quant)))
#endif

/* IDCT_range_limit stub: return a valid range table large enough for RANGE_MASK */
