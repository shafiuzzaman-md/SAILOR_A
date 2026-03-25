/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef ONE
#define ONE 1
#endif
#ifndef RANGE_CENTER
#define RANGE_CENTER 128
#endif
#ifndef RANGE_MASK
#define RANGE_MASK 1023  /* matches libjpeg typical MAXJSAMPLE*4+3 for 8-bit */
#endif
#ifndef DCTSIZE
#define DCTSIZE 8
#endif

/* Minimal type stand-ins to compile the harness */
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE;
typedef JSAMPLE* JSAMPROW;
typedef JSAMPROW* JSAMPARRAY;
typedef short JCOEF;
typedef JCOEF* JCOEFPTR;
typedef int INT32;
typedef int ISLOW_MULT_TYPE;

typedef struct jpeg_decompress_struct { int dummy; } jpeg_decompress_struct;
typedef jpeg_decompress_struct* j_decompress_ptr;

typedef struct jpeg_component_info { void *dct_table; } jpeg_component_info;

#ifndef RIGHT_SHIFT
#define RIGHT_SHIFT(x,shft) ((x) >> (shft))
#endif
#ifndef MULTIPLY
#define MULTIPLY(a,b) ((INT32)((a)*(b)))
#endif
#ifndef FIX
#define FIX(x) ((INT32)16384) /* dummy fixed-point constant */
#endif

/* Neutralized vulnerable function: keep signature and the vulnerable statement verbatim */
void jpeg_idct_3x3 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
               JCOEFPTR coef_block,
