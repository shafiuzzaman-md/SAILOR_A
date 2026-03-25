/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for jpeg_idct_12x6 */
#include <stdint.h>
#include <stdlib.h>

/* Minimal type defs to match signature */
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef short JCOEF; typedef JCOEF * JCOEFPTR;
typedef struct jpeg_decompress_struct { int dummy; } * j_decompress_ptr;

typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;

typedef int INT32;  /* sufficient for this harness */

#ifndef RIGHT_SHIFT
#define RIGHT_SHIFT(x,shft) ((x) >> (shft))
#endif
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef RANGE_MASK
#define RANGE_MASK 1023
#endif

