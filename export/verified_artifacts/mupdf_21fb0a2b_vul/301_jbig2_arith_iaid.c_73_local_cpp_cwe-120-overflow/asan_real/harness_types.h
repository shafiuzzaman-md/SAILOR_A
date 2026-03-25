/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Minimal local type/const definitions
#ifndef JBIG2_SEVERITY_FATAL
#define JBIG2_SEVERITY_FATAL 3
#endif
#ifndef JBIG2_UNKNOWN_SEGMENT_NUMBER
#define JBIG2_UNKNOWN_SEGMENT_NUMBER -1
#endif

typedef unsigned char byte;

typedef struct _Jbig2Allocator {
    int dummy;
} Jbig2Allocator;

typedef struct _Jbig2Ctx {
    Jbig2Allocator *allocator;
} Jbig2Ctx;

typedef struct _Jbig2ArithCx {
    uint8_t a;  // Make sizeof(Jbig2ArithCx) > 1 to enable alloc-size multiplication overflow
    uint8_t b;
} Jbig2ArithCx;

typedef struct _Jbig2ArithIaidCtx {
    uint8_t SBSYMCODELEN;
    Jbig2ArithCx *IAIDx;
} Jbig2ArithIaidCtx;

