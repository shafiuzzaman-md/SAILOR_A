/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local JPEG type defs to keep harness standalone
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef short JCOEF;               // typical in libjpeg
typedef JCOEF * JCOEFPTR;
typedef int INT32;

typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct;
typedef jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table;  // only to satisfy field presence; not dereferenced here
} jpeg_component_info;

typedef int ISLOW_MULT_TYPE;  // not used in our slice but declared for completeness

// Constants/macros used by the vulnerable expressions
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
#define RIGHT_SHIFT(x,sh) ((x) >> (sh))
#endif

// Provide a range_limit table like libjpeg would. Size ensures (.. & RANGE_MASK) is safe.
