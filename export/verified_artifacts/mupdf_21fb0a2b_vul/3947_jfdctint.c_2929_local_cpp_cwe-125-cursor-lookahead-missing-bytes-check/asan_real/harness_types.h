/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

/* Minimal local typedefs/macros to compile the slice */
typedef short DCTELEM;
typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;
typedef JSAMPROW * JSAMPARRAY;
typedef unsigned int JDIMENSION;
typedef int INT32;

#ifndef CENTERJSAMPLE
#define CENTERJSAMPLE 128
#endif
#ifndef PASS1_BITS
#define PASS1_BITS 2
#endif
#ifndef GETJSAMPLE
#define GETJSAMPLE(x) ((int)(x))
#endif

/* Vulnerable function (spine + sink) — neutralized to minimal body.
   Keep the vulnerable statement verbatim and add the universal sink assertion after it. */
