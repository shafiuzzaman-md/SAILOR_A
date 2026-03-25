/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for libpng write_unknown_chunks WMI-2 (type confusion/UAF)
 * Sliced and neutralized: entry is a direct pass-through, vulnerable function contains
 * the exact vulnerable statement and a UAF pattern (free before use).
 */
#include <stddef.h>
#include <stdlib.h>

/* Minimal local typedefs (pointers only cross TUs) */
typedef struct png_unknown_chunk {
    char name[5];
    unsigned char *data;
    size_t size;
    unsigned int location;
} png_unknown_chunk;

typedef struct png_struct_def {
    unsigned int mode;
    unsigned int mng_features_permitted;
    int unknown_default;
} png_struct;

typedef struct png_info_def {
    /* only the fields we touch in this slice */
    png_unknown_chunk *unknown_chunks;
    unsigned int unknown_chunks_num;
    /* many real fields omitted */
} png_info;

/* Forward decl per original signature */

/* ENTRY: neutralized pass-through (no guards, no other work) */
