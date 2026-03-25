/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for jpeg_idct_13x13 focusing on the vulnerable statement */
#include <stddef.h>
#include <stdint.h>

/* Minimal IJG-like typedefs to compile standalone */
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef short JCOEF;
typedef JCOEF * JCOEFPTR;
typedef int INT32;
typedef int ISLOW_MULT_TYPE;

typedef struct {
    void *dct_table; /* used by quantptr = (ISLOW_MULT_TYPE*)compptr->dct_table; */
} jpeg_component_info;

typedef struct jpeg_decompress_struct {
    JSAMPLE *sample_range_limit; /* IDCT_range_limit(cinfo) uses this */
} jpeg_decompress_struct;

typedef jpeg_decompress_struct * j_decompress_ptr;

/* Macros approximated from IJG for standalone build */
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef RANGE_MASK
#define RANGE_MASK 1023 /* (MAXJSAMPLE*4+3) for 8-bit samples */
#endif
#ifndef RIGHT_SHIFT
#define RIGHT_SHIFT(x,shft) ((x) >> (shft))
#endif
#ifndef SHIFT_TEMPS
#define SHIFT_TEMPS /* no temps needed in this sliced version */
#endif
#ifndef IDCT_range_limit
#define IDCT_range_limit(cinfo) ((cinfo)->sample_range_limit)
#endif

/* Entry == Vulnerable function: keep signature, minimal body, and exact vulnerable line */
void jpeg_idct_13x13 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		 JCOEFPTR coef_block,
