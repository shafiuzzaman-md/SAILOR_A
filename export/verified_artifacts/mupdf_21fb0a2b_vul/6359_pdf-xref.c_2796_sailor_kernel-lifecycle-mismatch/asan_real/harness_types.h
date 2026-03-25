/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal local type definitions to satisfy the harness
typedef struct fz_context { int dummy; } fz_context;

typedef struct pdf_xref {
    int num_objects;
} pdf_xref;

typedef struct pdf_document {
    struct pdf_xref *local_xref;
    int local_xref_nesting;
} pdf_document;

// Entry function: MUST be a direct pass-through to the vulnerable function (no guards)
