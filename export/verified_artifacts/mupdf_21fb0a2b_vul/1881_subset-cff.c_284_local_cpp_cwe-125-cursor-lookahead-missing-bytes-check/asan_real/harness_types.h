/* AUTO-GENERATED from harness preamble */
#pragma once

// harness/spine.c
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef FZ_ERROR_FORMAT
#define FZ_ERROR_FORMAT (-1)
#endif

// Minimal type defs needed by the slice
typedef unsigned int offsize_t;

typedef struct {
    uint32_t index_offset;
    uint32_t count;
    uint8_t offsize;
    const uint8_t *offset;
    uint32_t data_offset;
    uint32_t index_size;
} index_t;

typedef struct {
    unsigned char *data;
    size_t len;
} fz_buffer;

typedef struct fz_context { int dummy; } fz_context;

// Simple helpers (big-endian)
