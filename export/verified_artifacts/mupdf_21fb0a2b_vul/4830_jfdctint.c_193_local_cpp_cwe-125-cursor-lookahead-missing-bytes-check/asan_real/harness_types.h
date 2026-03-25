/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for jpeg_fdct_islow targeting jfdctint.c:193 */
#include <stdint.h>
#include <stddef.h>

#ifndef DCTSIZE
#define DCTSIZE 8
#endif

typedef unsigned char JSAMPLE;
typedef JSAMPLE *JSAMPROW;
typedef JSAMPROW *JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef int INT32;
typedef int DCTELEM;

#ifndef GETJSAMPLE
#define GETJSAMPLE(x) ((int)(x))
#endif

