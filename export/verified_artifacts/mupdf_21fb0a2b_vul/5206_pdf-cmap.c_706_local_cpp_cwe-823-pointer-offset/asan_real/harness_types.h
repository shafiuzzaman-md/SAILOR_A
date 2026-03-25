/* AUTO-GENERATED from harness preamble */
#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Minimal type definitions
typedef struct fz_context_s fz_context;  // opaque

typedef struct pdf_cmap_s {
    int *dict;
    int dlen;
    int dcap;
    char cmap_name[32];
} pdf_cmap;

// Realloc helper matching MuPDF-style macro
#ifndef fz_realloc_array
#define fz_realloc_array(ctx, p, n, type) ( (type *)realloc((p), sizeof(type) * (n)) )
#endif

// Entry function: MUST be a direct pass-through to vulnerable function
