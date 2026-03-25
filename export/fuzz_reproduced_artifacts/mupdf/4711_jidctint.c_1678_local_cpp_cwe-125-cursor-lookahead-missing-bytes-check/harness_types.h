/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for jpeg_idct_11x11 focusing on the vulnerable statement.
 * We define only the needed types/macros and keep the vulnerable line verbatim.
 */
#include <stddef.h>
#include <stdint.h>

/* Minimal JPEG type defs sufficient for this slice */
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef int INT32;
typedef int ISLOW_MULT_TYPE;

typedef struct jpeg_decompress_struct {
    JSAMPLE *sample_range_limit; /* used by IDCT_range_limit(cinfo) */
} jpeg_decompress_struct;

typedef jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table; /* entry precondition mentions this field */
} jpeg_component_info;

typedef short * JCOEFPTR;

/* Macros/constants used by the vulnerable statement */
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef RIGHT_SHIFT
#define RIGHT_SHIFT(x,shft) ((x) >> (shft))
#endif
#ifndef RANGE_MASK
#define RANGE_MASK 1023
#endif

#ifndef IDCT_range_limit
#define IDCT_range_limit(cinfo) ((cinfo)->sample_range_limit)
#endif

/* Neutralized version of jpeg_idct_11x11 containing only what's needed
 * to reach and trigger the vulnerable access. */
void jpeg_idct_11x11 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                      JCOEFPTR coef_block,
