/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

/* Minimal local JPEG-like typedefs/macros to compile standalone */
typedef short DCTELEM;
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef int INT32;

#ifndef GETJSAMPLE
#define GETJSAMPLE(value) ((int)(value))
#endif

#ifndef SHIFT_TEMPS
#define SHIFT_TEMPS
#endif

/* Neutralized vulnerable function containing only the path to the sink */
