/* AUTO-GENERATED from harness preamble */
#pragma once

/* spine_harness.c - minimal sliced harness for jpeg_idct_12x12 */
#include <stdint.h>
#include <stdlib.h>

// Minimal local typedefs to avoid pulling full libjpeg headers
typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct;
typedef jpeg_decompress_struct * j_decompress_ptr;

typedef unsigned int JDIMENSION;
typedef short JCOEF; typedef JCOEF * JCOEFPTR;
typedef unsigned char JSAMPLE; typedef JSAMPLE * JSAMPROW; typedef JSAMPROW * JSAMPARRAY;

typedef int INT32;
typedef int ISLOW_MULT_TYPE;  // minimal stand-in

typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;

// Vulnerable function (neutralized: keep only what's needed to reach sink)
static void vul_jpeg_idct_12x12(j_decompress_ptr cinfo, jpeg_component_info * compptr,
