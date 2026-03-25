/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

// Minimal type defs to satisfy the signature
typedef void * j_decompress_ptr;
typedef unsigned int JDIMENSION;
typedef short JCOEF; typedef JCOEF * JCOEFPTR;
typedef unsigned char JSAMPLE; typedef JSAMPLE * JSAMPROW; typedef JSAMPROW * JSAMPARRAY;

typedef struct jpeg_component_info { void * dct_table; } jpeg_component_info;

// Neutralized vulnerable function body: keep signature and the vulnerable statement verbatim
void jpeg_idct_islow (j_decompress_ptr cinfo, jpeg_component_info * compptr,
		 JCOEFPTR coef_block,
