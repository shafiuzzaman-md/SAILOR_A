/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

/* Minimal local JPEG type/macro definitions */
#ifndef DCTSIZE
#define DCTSIZE 8
#endif
#ifndef JSAMPLE
typedef unsigned char JSAMPLE;
#endif
#ifndef JSAMPROW
typedef JSAMPLE *JSAMPROW;
#endif
#ifndef JSAMPARRAY
typedef JSAMPROW *JSAMPARRAY;
#endif
#ifndef JDIMENSION
typedef unsigned int JDIMENSION;
#endif
#ifndef DCTELEM
typedef short DCTELEM;
#endif

#ifndef CENTERJSAMPLE
#define CENTERJSAMPLE 128
#endif

#ifndef GETJSAMPLE
#define GETJSAMPLE(x) ((int)(x))
#endif

#ifndef RIGHT_SHIFT
#define RIGHT_SHIFT(x,n) ((x) >> (n))
#endif

#ifndef DESCALE
#define DESCALE(x,n) RIGHT_SHIFT((x) + (1 << ((n)-1)), (n))
#endif

#ifndef CONST_BITS
#define CONST_BITS 8
#endif

#if CONST_BITS == 8
#define FIX_0_382683433  ((int32_t)   98)
#define FIX_0_541196100  ((int32_t)  139)
#define FIX_0_707106781  ((int32_t)  181)
#define FIX_1_306562965  ((int32_t)  334)
#else
#define FIX_0_382683433  FIX(0.382683433)
#define FIX_0_541196100  FIX(0.541196100)
#define FIX_0_707106781  FIX(0.707106781)
#define FIX_1_306562965  FIX(1.306562965)
#endif

#ifndef MULTIPLY
#define MULTIPLY(var,const)  ((DCTELEM) DESCALE((var) * (const), CONST_BITS))
#endif

#ifndef SHIFT_TEMPS
#define SHIFT_TEMPS /* no temps needed in this slice */
#endif

/* Entry == vulnerable function. Keep minimal path to the vulnerable statement. */
