/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local jpeg types to match signature without including jpeglib.h
struct jpeg_decompress_struct; // opaque
typedef struct jpeg_decompress_struct * j_decompress_ptr;

typedef unsigned int JDIMENSION;
typedef short JCOEF;
typedef JCOEF * JCOEFPTR;
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;

typedef struct jpeg_component_info {
    void *dct_table;   // we only need this field for quantptr/wsptr source
    int DCT_scaled_size;
} jpeg_component_info;

typedef int INT32;
typedef int ISLOW_MULT_TYPE;

// Local constants/macros (values borrowed from IJG typical defaults)
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef ONE
#define ONE 1
#endif
#ifndef CENTERJSAMPLE
#define CENTERJSAMPLE 128
#endif
#ifndef RANGE_CENTER
#define RANGE_CENTER CENTERJSAMPLE
#endif
#ifndef DCTSIZE
#define DCTSIZE 8
#endif

// Fixed-point coefficients (IJG common values with CONST_BITS=13)
#ifndef FIX_0_541196100
#define FIX_0_541196100 4433
#endif
#ifndef FIX_0_765366865
#define FIX_0_765366865 6270
#endif
#ifndef FIX_1_847759065
#define FIX_1_847759065 15137
#endif

#ifndef MULTIPLY
#define MULTIPLY(var, constval)  ((INT32)(var) * (INT32)(constval))
#endif

// IDCT_range_limit stub: provide a stable table independent of cinfo
