/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for adler32.c target around line 118 */
#include <stdint.h>
#include <stdlib.h>

/* Local zlib-like typedefs/macros to avoid including full zlib */
typedef unsigned long uLong;
typedef unsigned int uInt;
typedef unsigned char Bytef;

#ifndef ZEXPORT
#define ZEXPORT
#endif

/* Constants/macros reconstructed from adler32.c */
#ifndef BASE
#define BASE 65521U /* largest prime smaller than 65536 */
#endif
#ifndef NMAX
#define NMAX 5552
#endif

#define DO1(buf,i)  { adler += (buf)[(i)]; sum2 += adler; }
#define DO2(buf,i)  DO1(buf,i); DO1(buf,(i)+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,(i)+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,(i)+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);
