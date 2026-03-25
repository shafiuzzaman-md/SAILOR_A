/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local typedefs to satisfy signature
#ifndef JSAMPLE_T
#define JSAMPLE_T
typedef unsigned char JSAMPLE;
typedef JSAMPLE *JSAMPROW;
typedef JSAMPROW *JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef int INT32;
typedef short JCOEF;
typedef JCOEF * JCOEFPTR;
typedef int ISLOW_MULT_TYPE;

typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct;
typedef jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;
#endif

// Minimal macros used by the vulnerable expression
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef RANGE_MASK
#define RANGE_MASK 1023
#endif
#ifndef RIGHT_SHIFT
#define RIGHT_SHIFT(x,n) ((x) >> (n))
#endif

// Provide a range limit table like libjpeg expects
