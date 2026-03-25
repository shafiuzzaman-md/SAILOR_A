/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

// Minimal type/macro definitions to compile the sliced function
typedef unsigned char JSAMPLE;
typedef JSAMPLE* JSAMPROW;
typedef JSAMPROW* JSAMPARRAY;
typedef int JDIMENSION;
typedef int INT32;
typedef short JCOEF;
typedef JCOEF* JCOEFPTR;

typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct;
typedef jpeg_decompress_struct* j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table;
} jpeg_component_info;

typedef int ISLOW_MULT_TYPE;

#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef RANGE_MASK
#define RANGE_MASK 1023
#endif
#ifndef ONE
#define ONE 1
#endif
#ifndef SHIFT_TEMPS
#define SHIFT_TEMPS
#endif
#ifndef RIGHT_SHIFT
#define RIGHT_SHIFT(x,shft) ((x) >> (shft))
#endif

// Minimal range limit table used by IDCT_range_limit
