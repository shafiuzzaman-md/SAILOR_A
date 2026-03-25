/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef UINT32_MAX
#define UINT32_MAX 0xFFFFFFFFu
#endif

typedef unsigned char byte;

typedef struct {
    uint32_t data_length;
    uint32_t number;
    uint32_t rows;
} Jbig2Segment;

typedef struct {
    int dummy;
} Jbig2Ctx;

// Treat arithmetic context as a byte buffer
typedef unsigned char Jbig2ArithCx;

// ENTRY == VUL FUNC: keep only the vulnerable statement
