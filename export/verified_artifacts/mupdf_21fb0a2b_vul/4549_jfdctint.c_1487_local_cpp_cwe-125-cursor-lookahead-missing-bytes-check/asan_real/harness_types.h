/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local typedefs/macros to compile the slice
#ifndef DCTELEM
typedef short DCTELEM;
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
#ifndef INT32
typedef int INT32;
#endif

#ifndef CENTERJSAMPLE
#define CENTERJSAMPLE 128
#endif
#ifndef GETJSAMPLE
#define GETJSAMPLE(x) ((int)(x))
#endif
#ifndef SHIFT_TEMPS
#define SHIFT_TEMPS
#endif

// Neutralized vulnerable function (spine == entry == vul func)
