/* AUTO-GENERATED from harness preamble */
#pragma once


#include <stdint.h>

#ifndef GLOBAL
#define GLOBAL(type) type
#endif

/* Minimal local type definitions to compile standalone */
typedef short DCTELEM;
typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE;
typedef JSAMPLE *JSAMPROW;
typedef JSAMPROW *JSAMPARRAY;
typedef int INT32;

/* Minimal macro definitions used by the vulnerable statement */
#ifndef ONE
#define ONE 1
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef RIGHT_SHIFT
#define RIGHT_SHIFT(x,sh) ((x) >> (sh))
#endif
#ifndef MULTIPLY
#define MULTIPLY(var,const_) ((INT32)((var) * (const_)))
#endif

/* Fixed-point constants (values from common libjpeg integer FDCT config) */
#ifndef FIX_0_541196100
#define FIX_0_541196100  ((INT32)  4433)   /* FIX(0.541196100) */
#endif
#ifndef FIX_0_765366865
#define FIX_0_765366865  ((INT32)  6270)   /* FIX(0.765366865) */
#endif
#ifndef FIX_1_847759065
#define FIX_1_847759065  ((INT32)  15137)  /* FIX(1.847759065) */
#endif

/* Entry == vulnerable function. Keep only the path needed to reach the sink. */
GLOBAL(void)
