/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for jpeg_idct_2x2 */
#include <stdint.h>
#include <stdlib.h>

/* Local JPEG-ish typedefs to satisfy signature */
typedef unsigned int JDIMENSION;
typedef short DCTELEM;
typedef short JCOEF;
typedef JCOEF* JCOEFPTR;
typedef unsigned char JSAMPLE;
typedef JSAMPLE* JSAMPROW;
typedef JSAMPROW* JSAMPARRAY;

typedef struct jpeg_decompress_struct { int dummy; } * j_decompress_ptr;

typedef int ISLOW_MULT_TYPE;

#ifndef DCTSIZE
#define DCTSIZE 8
#endif

#ifndef RANGE_CENTER
#define RANGE_CENTER 128
#endif

#ifndef RANGE_MASK
/* Large mask to keep index within table size below (2048-1) */
#define RANGE_MASK 2047
#endif

#ifndef IRIGHT_SHIFT
#define IRIGHT_SHIFT(x,shft) ((x) >> (shft))
#endif

#ifndef ISHIFT_TEMPS
#define ISHIFT_TEMPS /* no temps needed here */
#endif

#ifndef DEQUANTIZE
#define DEQUANTIZE(coef,quant) ((DCTELEM)((coef) * (quant)))
#endif

