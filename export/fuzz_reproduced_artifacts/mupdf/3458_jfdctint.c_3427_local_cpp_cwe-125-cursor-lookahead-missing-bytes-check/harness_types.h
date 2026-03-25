/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for jpeg_fdct_8x16 focusing on the vulnerable read */
#include <stddef.h>
#include <stdint.h>

/* Local minimal type/macro defs to compile standalone */
#ifndef DCTELEM
#define DCTELEM int
#endif
#ifndef INT32
#define INT32 int
#endif
#ifndef JSAMPLE
#define JSAMPLE unsigned char
#endif
#ifndef JSAMPROW
#define JSAMPROW JSAMPLE*
#endif
#ifndef JSAMPARRAY
#define JSAMPARRAY JSAMPROW*
#endif
#ifndef JDIMENSION
#define JDIMENSION unsigned int
#endif
#ifndef DCTSIZE2
#define DCTSIZE2 64
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
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

/* Vulnerable function — keep signature and the exact vulnerable line */
