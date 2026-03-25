/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for libpng png_set_PLTE vulnerability (pngset.c:751)
 * We keep only the vulnerable dereference and add a universal sink assertion.
 */
#include <stdint.h>
#include <stdlib.h>

/* Local minimal type/macro definitions to satisfy the signature and code */
#ifndef PNG_COLOR_TYPE_PALETTE
#define PNG_COLOR_TYPE_PALETTE 3
#endif
#ifndef PNG_MAX_PALETTE_LENGTH
#define PNG_MAX_PALETTE_LENGTH 256
#endif

typedef uint32_t png_uint_32;

typedef struct png_color_s {
    unsigned char red, green, blue;
} png_color;

typedef struct png_struct_def {
    unsigned int mng_features_permitted; /* not used in our minimal path */
} png_struct;

typedef struct png_info_def {
    int color_type;
    int bit_depth;
} png_info;

/* Neutralized vulnerable function: keep signature and the exact vulnerable line */
void png_set_PLTE(png_struct *png_ptr, png_info *info_ptr,
