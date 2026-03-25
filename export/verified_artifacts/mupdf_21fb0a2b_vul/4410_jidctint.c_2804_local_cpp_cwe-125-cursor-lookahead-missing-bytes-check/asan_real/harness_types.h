/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>

typedef unsigned int JDIMENSION;
typedef int INT32;
typedef int ISLOW_MULT_TYPE;
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;
typedef short JCOEF;
typedef JCOEF * JCOEFPTR;

struct jpeg_decompress_struct { int dummy; };
typedef struct jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info {
    void *dct_table;
} jpeg_component_info;

// Neutralized vulnerable function with the exact vulnerable statement from jidctint.c:2804
void jpeg_idct_16x16 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                      JCOEFPTR coef_block,
