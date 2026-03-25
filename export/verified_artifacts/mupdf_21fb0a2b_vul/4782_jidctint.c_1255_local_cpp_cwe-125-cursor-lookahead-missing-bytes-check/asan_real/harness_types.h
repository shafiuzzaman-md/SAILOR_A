/* AUTO-GENERATED from harness preamble */
#pragma once

/* minimal sliced harness for jpeg_idct_9x9 */
#include <stdlib.h>
#include <stdint.h>

/* Minimal local typedefs to satisfy signature */
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE; typedef JSAMPLE *JSAMPROW; typedef JSAMPROW *JSAMPARRAY;
typedef short JCOEF; typedef JCOEF *JCOEFPTR;
typedef int INT32;  /* match usage in source */

typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct; /* opaque */
typedef jpeg_decompress_struct * j_decompress_ptr;

typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;

typedef int DCTELEM; /* workspace element type for simplicity */

/* Entry == vulnerable function. Keep signature, minimal body with target line. */
int jpeg_idct_9x9(j_decompress_ptr cinfo, jpeg_component_info * compptr,
