/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>

// Minimal local typedefs/macros to satisfy compilation
#ifndef DCTELEM
typedef int16_t DCTELEM;
#endif
#ifndef JDIMENSION
typedef unsigned int JDIMENSION;
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
#ifndef INT32
typedef int32_t INT32;
#endif
#ifndef DCTSIZE
#define DCTSIZE 8
#endif
#ifndef CENTERJSAMPLE
#define CENTERJSAMPLE 128
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef SHIFT_TEMPS
#define SHIFT_TEMPS /* none */
#endif
#ifndef GETJSAMPLE
#define GETJSAMPLE(x) ((int)(x))
#endif
#ifndef FIX
#define FIX(x) ((INT32) ((x) * (1 << CONST_BITS) + 0.5))
#endif
#ifndef MULTIPLY
#define MULTIPLY(var, c) ((var) * (c))
#endif
#ifndef DESCALE
#define DESCALE(x, n) ((x) >> (n))
#endif
#ifndef FIX_0_541196100
#define FIX_0_541196100 FIX(0.541196100)
#endif

// Entry function: mandatory direct pass-through to vulnerable function
