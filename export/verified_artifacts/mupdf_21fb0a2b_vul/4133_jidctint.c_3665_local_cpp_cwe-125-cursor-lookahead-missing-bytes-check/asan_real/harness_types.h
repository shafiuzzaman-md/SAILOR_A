/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifndef INT32
#define INT32 int
#endif
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef FIX
#define FIX(x) ((INT32)((x) * (1 << CONST_BITS) + 0.5))
#endif
#ifndef MULTIPLY
#define MULTIPLY(var,constval) ((INT32)((var) * (constval)))
#endif

/* Minimal local type shims to satisfy signature */
typedef struct jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table; /* only field referenced by preconditions */
} jpeg_component_info;

typedef short JCOEF; typedef JCOEF * JCOEFPTR;
typedef unsigned char JSAMPLE; typedef JSAMPLE * JSAMPROW; typedef JSAMPROW * JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef int ISLOW_MULT_TYPE;

/* Neutralized spine containing only the target case and vulnerable statement. */
void jpeg_idct_10x5(j_decompress_ptr cinfo, jpeg_component_info * compptr,
