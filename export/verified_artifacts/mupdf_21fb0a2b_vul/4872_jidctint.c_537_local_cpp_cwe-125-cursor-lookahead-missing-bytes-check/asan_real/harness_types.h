/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local type defs to match signature
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPLE ** JSAMPARRAY;
typedef short JCOEF;
typedef JCOEF * JCOEFPTR;

typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct;
typedef jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;

typedef int INT32;

// Vulnerable function (entry == vulnerable). Keep signature and the vulnerable line verbatim.
void jpeg_idct_7x7 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
