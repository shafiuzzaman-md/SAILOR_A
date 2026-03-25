/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal local typedefs to match the signature */
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;
typedef short JCOEF;
typedef JCOEF * JCOEFPTR;
typedef int INT32;
typedef int ISLOW_MULT_TYPE;

typedef struct jpeg_decompress_struct { int dummy; } * j_decompress_ptr;
typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;

/* Macros/constants used by the vulnerable lines */
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef RANGE_CENTER
#define RANGE_CENTER 128
#endif
#ifndef RANGE_MASK
#define RANGE_MASK 255
#endif
#ifndef SHIFT_TEMPS
#define SHIFT_TEMPS /* no-op */
#endif
#ifndef RIGHT_SHIFT
#define RIGHT_SHIFT(x,shft) ((INT32)((x) >> (shft)))
#endif

