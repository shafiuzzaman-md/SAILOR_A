/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/* Minimal local definitions to compile the slice */
#ifndef DCTSIZE
#define DCTSIZE 8
#endif

#ifndef CENTERJSAMPLE
#define CENTERJSAMPLE 128
#endif

typedef unsigned int JDIMENSION;
typedef unsigned char JSAMPLE;
typedef JSAMPLE *JSAMPROW;
typedef JSAMPROW *JSAMPARRAY;
typedef short DCTELEM;     /* enough for IJG's fast DCT */
typedef int INT32;

/* Shift/scale helpers (simplified) */
#define SHIFT_TEMPS
#ifndef RIGHT_SHIFT
#define RIGHT_SHIFT(x,n)  ((x) >> (n))
#endif
#ifndef DESCALE
#define DESCALE(x,n)  RIGHT_SHIFT((x) + (1 << ((n)-1)), (n))
#endif

#define CONST_BITS 8
#if CONST_BITS == 8
#define FIX_0_382683433  ((INT32)   98)
#define FIX_0_541196100  ((INT32)  139)
#define FIX_0_707106781  ((INT32)  181)
#define FIX_1_306562965  ((INT32)  334)
#else
#define FIX_0_382683433  FIX(0.382683433)
#define FIX_0_541196100  FIX(0.541196100)
#define FIX_0_707106781  FIX(0.707106781)
#define FIX_1_306562965  FIX(1.306562965)
#endif

#ifndef GETJSAMPLE
#define GETJSAMPLE(x) ((int)(x))
#endif

#define MULTIPLY(var,const)  ((DCTELEM) DESCALE((var) * (const), CONST_BITS))

/* Entry == vulnerable function: keep minimal path to the vulnerable statement */
