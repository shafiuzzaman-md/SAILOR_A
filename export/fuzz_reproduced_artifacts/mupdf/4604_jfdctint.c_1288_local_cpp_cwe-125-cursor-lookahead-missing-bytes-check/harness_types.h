/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local typedefs/macros (project-independent)
#ifndef JDIMENSION
typedef unsigned int JDIMENSION;
#endif
#ifndef JSAMPLE
typedef unsigned char JSAMPLE;
#endif
#ifndef JSAMPROW
typedef JSAMPLE * JSAMPROW;
#endif
#ifndef JSAMPARRAY
typedef JSAMPROW * JSAMPARRAY;
#endif
#ifndef DCTELEM
typedef short DCTELEM;
#endif
#ifndef INT32
typedef int INT32;
#endif

#ifndef SHIFT_TEMPS
#define SHIFT_TEMPS /* no-op for harness */
#endif
#ifndef GETJSAMPLE
#define GETJSAMPLE(x) ((int)(x))
#endif
#ifndef CENTERJSAMPLE
#define CENTERJSAMPLE 128
#endif
#ifndef CONST_BITS
#define CONST_BITS 13
#endif
#ifndef FIX
#define FIX(x) ((int)((x) * (1 << CONST_BITS) + 0.5))
#endif
#ifndef MULTIPLY
#define MULTIPLY(a,b) ((int)((a) * (b)))
#endif
#ifndef DESCALE
#define DESCALE(x,n) ((int)((x) >> (n)))
#endif

// Entry == vulnerable function (pass-through rule satisfied implicitly)
