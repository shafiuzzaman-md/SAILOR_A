/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/* Minimal local type defs to avoid pulling project headers */
typedef unsigned char png_byte;
typedef struct png_struct_def png_struct; /* opaque */

typedef struct png_sPLT_entry_s {
    unsigned int red;
    unsigned int green;
    unsigned int blue;
    unsigned int alpha;
    unsigned int frequency;
} png_sPLT_entry;

typedef struct png_sPLT_s {
    const char *name;
    png_byte depth;
    png_sPLT_entry *entries;
    unsigned int nentries;
} png_sPLT_t;

/* Vulnerable function: keep signature and the exact vulnerable line; add universal sink assertion after it */
