/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <string.h>

// Minimal type definitions needed for the slice
typedef struct fz_context { int dummy; } fz_context;

typedef void (*fz_output_write_fn)(fz_context *ctx, void *state, const void *data, size_t size);

typedef struct fz_output {
    char *bp;   // buffer start
    char *wp;   // write pointer
    char *ep;   // buffer end
    void *state;
    fz_output_write_fn write;
} fz_output;

typedef struct fz_buffer {
    unsigned char *data;
    size_t len;
} fz_buffer;

// Vulnerable function: keep body with the exact vulnerable statement and add sink assertion after it
