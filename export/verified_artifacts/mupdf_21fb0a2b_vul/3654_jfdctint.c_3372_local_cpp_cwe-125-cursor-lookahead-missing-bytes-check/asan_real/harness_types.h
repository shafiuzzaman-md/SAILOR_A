/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for jpeg_fdct_2x1 */
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Minimal local typedefs/macros to compile standalone */
typedef int DCTELEM;              /* use int for arithmetic headroom */
typedef unsigned int JDIMENSION;  /* reasonable stand-in */
typedef unsigned char JSAMPLE;
typedef JSAMPLE *JSAMPROW;
typedef JSAMPROW *JSAMPARRAY;

#ifndef DCTSIZE2
#define DCTSIZE2 64
#endif
#ifndef CENTERJSAMPLE
#define CENTERJSAMPLE 128
#endif
#ifndef SIZEOF
#define SIZEOF(x) sizeof(x)
#endif
#ifndef MEMZERO
#define MEMZERO(target, size) memset((void *)(target), 0, (size))
#endif
#ifndef GETJSAMPLE
#define GETJSAMPLE(value) ((int)(value))
#endif

