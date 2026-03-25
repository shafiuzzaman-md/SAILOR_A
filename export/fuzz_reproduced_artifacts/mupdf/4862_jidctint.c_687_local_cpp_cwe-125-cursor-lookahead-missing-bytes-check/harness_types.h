/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for jpeg_idct_6x6 */
#include <stdint.h>
#include <stdlib.h>

// Minimal local type defs to avoid heavy includes
typedef unsigned int JDIMENSION;

typedef unsigned char JSAMPLE;    // from libjpeg
typedef JSAMPLE *JSAMPROW;
typedef JSAMPLE **JSAMPARRAY;

typedef short JCOEF;              // typical libjpeg
typedef JCOEF * JCOEFPTR;

typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct;
typedef jpeg_decompress_struct * j_decompress_ptr;

typedef short ISLOW_MULT_TYPE;    // dequant table entry type (is typically int16)

typedef struct jpeg_component_info {
    void *dct_table;              // only field we care about for precondition
} jpeg_component_info;

typedef int INT32;

// Entry == vulnerable function. Keep signature, include only target statement and sink.
void jpeg_idct_6x6 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                    JCOEFPTR coef_block,
