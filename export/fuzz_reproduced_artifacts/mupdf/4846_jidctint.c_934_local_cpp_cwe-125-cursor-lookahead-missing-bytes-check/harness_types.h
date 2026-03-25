/* AUTO-GENERATED from harness preamble */
#pragma once


// Minimal sliced harness for jpeg_idct_4x4 targeting jidctint.c:934
#include <stdint.h>
#include <stdlib.h>

// Minimal type defs to satisfy signature
typedef unsigned char JSAMPLE;
typedef JSAMPLE *JSAMPROW;
typedef JSAMPROW *JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef short JCOEF;
typedef JCOEF * JCOEFPTR;

typedef struct jpeg_decompress_struct { int dummy; } * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table;
} jpeg_component_info;

// Minimal constants/macros used by the vulnerable statement
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef RANGE_MASK
#define RANGE_MASK 1023
#endif
#ifndef RIGHT_SHIFT
#define RIGHT_SHIFT(x,shft) ((x) >> (shft))
#endif

typedef int INT32;

global_static_range_limit:

// Neutralized vulnerable function: keep signature, declare locals, set up outptr/range_limit, hit sink
void jpeg_idct_4x4 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                    JCOEFPTR coef_block,
