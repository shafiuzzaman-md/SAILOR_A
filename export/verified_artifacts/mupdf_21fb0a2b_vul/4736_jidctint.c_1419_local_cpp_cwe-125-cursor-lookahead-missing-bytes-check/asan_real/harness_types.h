/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal local type/model definitions matching libjpeg style
#ifndef JDIMENSION
#define JDIMENSION unsigned int
#endif

typedef int INT32;  // simplify for harness

typedef struct jpeg_decompress_struct { int dummy; } * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table;  // used for quantptr/wsptr base in this harness
} jpeg_component_info;

typedef short JCOEF;           // typical libjpeg
typedef JCOEF * JCOEFPTR;      // coef pointer type

typedef unsigned char JSAMPLE; // sample type
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;

typedef INT32 ISLOW_MULT_TYPE; // for cast in preconditions

// Local constants/macros to compile the kept lines
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef RANGE_CENTER
#define RANGE_CENTER 128
#endif
#ifndef ONE
#define ONE 1
#endif
#ifndef FIX
#define FIX(x) ((INT32)((x) * (1 << CONST_BITS) + 0.5))
#endif
#ifndef MULTIPLY
#define MULTIPLY(x, y) ((INT32)(((INT64)(x) * (INT64)(y)) >> CONST_BITS))
#endif

// Entry == Vulnerable function. Keep signature. Neutralized body keeping only the path and vulnerable statement.
void jpeg_idct_10x10(j_decompress_ptr cinfo, jpeg_component_info * compptr,
