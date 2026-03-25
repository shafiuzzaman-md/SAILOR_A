/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness spine for pdf_map_one_to_many -> add_mrange */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Minimal opaque context */
typedef struct fz_context_s { int dummy; } fz_context;

typedef struct pdf_cmap_s {
    int dlen;
    int dcap;
    int *dict;
} pdf_cmap;

/* MuPDF-style realloc array macro simplified for ints */
#ifndef fz_realloc_array
#define fz_realloc_array(ctx, ptr, count, type) (int*)realloc((ptr), sizeof(int) * (count))
#endif

