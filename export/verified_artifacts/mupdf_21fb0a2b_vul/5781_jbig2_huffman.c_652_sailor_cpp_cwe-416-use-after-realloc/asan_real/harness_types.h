/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for jbig2_table UAF pattern with exact vulnerable statement */
#include <stdint.h>
#include <stdlib.h>

#ifndef LOG_TABLE_SIZE_MAX
#define LOG_TABLE_SIZE_MAX 16
#endif

typedef unsigned char byte;

typedef struct { int dummy; } Jbig2Ctx;

typedef struct {
    uint32_t number;
    size_t data_length;
    void *result;
} Jbig2Segment;

typedef struct {
    int PREFLEN;
    int RANGELEN;
    int32_t RANGELOW;
} Jbig2HuffmanLine;

typedef struct {
    int HTOOB;
    size_t n_lines;
    Jbig2HuffmanLine *lines;
} Jbig2HuffmanParams;

