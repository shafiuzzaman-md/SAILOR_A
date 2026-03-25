/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for MuPDF password auth vulnerability */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Minimal type defs to satisfy field accesses */
typedef struct fz_context { int dummy; } fz_context;

typedef struct pdf_crypt {
    int r; /* revision field referenced by original code, not used in slice */
} pdf_crypt;

typedef struct pdf_document {
    pdf_crypt *crypt;
} pdf_document;

/* ENTRY: Neutralized pass-through per rules */

