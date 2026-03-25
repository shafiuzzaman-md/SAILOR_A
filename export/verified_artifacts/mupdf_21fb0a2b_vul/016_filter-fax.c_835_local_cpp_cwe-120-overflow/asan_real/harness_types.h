/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Minimal stand-ins for MuPDF types
typedef struct fz_context { int dummy; } fz_context;
typedef struct fz_stream { int dummy; } fz_stream;

// Minimal subset of fz_faxd used on the path
typedef struct {
    fz_stream *chain;

    int k;
    int end_of_line;
    int encoded_byte_align;
    int columns;
    int rows;
    int end_of_block;
    int black_is_1;

    int stride;
    int ridx;

    int bidx;
    unsigned int word;

    int stage;

    int a, c, dim, eolc;
    unsigned char *ref;
    unsigned char *dst;
    unsigned char *rp, *wp;
} fz_faxd;

// Neutralized vulnerable function: keep signature and the vulnerable statements exactly
fz_stream *
fz_open_faxd(fz_context *ctx, fz_stream *chain,
    int k, int end_of_line, int encoded_byte_align,
