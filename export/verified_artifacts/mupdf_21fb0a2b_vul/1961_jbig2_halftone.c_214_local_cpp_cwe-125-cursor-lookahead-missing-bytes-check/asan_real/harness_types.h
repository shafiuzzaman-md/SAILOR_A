/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c - sliced harness for jbig2_pattern_dictionary */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* Minimal local type defs to satisfy the sliced code */
typedef unsigned char byte;

typedef struct {
    void *allocator; /* unused in slice */
} Jbig2Ctx;

typedef struct {
    uint32_t data_length;
    uint32_t number;
    void *result;
} Jbig2Segment;

typedef struct {
    int HDMMR;
    int HDTEMPLATE;
    int HDPW;
    int HDPH;
    uint32_t GRAYMAX;
} Jbig2PatternDictParams;

/* Sliced vulnerable function body (impl) with the exact vulnerable statement retained */
