/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for jpeg_fdct_9x9 focusing on the vulnerable statement */
#include <stdint.h>
#include <stddef.h>

/* Local typedefs/macros to avoid pulling full project headers */
#ifndef DCTELEM
typedef short DCTELEM;  /* typical for libjpeg */
#endif
#ifndef JSAMPLE
typedef unsigned char JSAMPLE;
#endif
#ifndef JSAMPROW
typedef JSAMPLE* JSAMPROW;
#endif
#ifndef JSAMPARRAY
typedef JSAMPROW* JSAMPARRAY;
#endif
#ifndef JDIMENSION
typedef unsigned int JDIMENSION;
#endif
#ifndef INT32
typedef int INT32;
#endif

#ifndef DCTSIZE
#define DCTSIZE 8
#endif
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef CONST_SCALE
#define CONST_SCALE (1 << CONST_BITS)
#endif
#ifndef FIX
#define FIX(x) ((INT32)((x) * (double)CONST_SCALE + 0.5))
#endif
#ifndef GETJSAMPLE
#define GETJSAMPLE(v) ((int)(v))
#endif
#ifndef DESCALE
#define DESCALE(x,n) ((x) >> (n))
#endif
#ifndef MULTIPLY
#define MULTIPLY(var,constv) ((var) * (constv))
#endif
#ifndef SHIFT_TEMPS
#define SHIFT_TEMPS /* no-op in this harness */
#endif

/* Entry == Vulnerable function: keep only the path to the sink, strip other logic */
