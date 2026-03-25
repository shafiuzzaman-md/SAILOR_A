/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for libpng: pngrtran.c path
 * Spine: png_do_read_transformations -> png_do_unshift
 */
#include <stddef.h>
#include <stdint.h>

/* Minimal local typedefs/macros to avoid pulling project headers */
typedef unsigned char png_byte;

typedef struct png_color_8 {
    png_byte red, green, blue, gray, alpha;
} png_color_8;

typedef struct png_row_info {
    unsigned int width;
    unsigned int rowbytes;
    png_byte color_type;
    png_byte bit_depth;
    png_byte pixel_depth;
} png_row_info;

typedef struct png_struct {
    png_byte *row_buf;
    unsigned int flags;
    unsigned int transformations;
    struct png_color_8 sig_bit;
} png_struct;

/* Color/alpha masks and types (minimal) */
#ifndef PNG_COLOR_MASK_COLOR
#define PNG_COLOR_MASK_COLOR 0x02
#endif
#ifndef PNG_COLOR_MASK_ALPHA
#define PNG_COLOR_MASK_ALPHA 0x04
#endif
#ifndef PNG_COLOR_TYPE_PALETTE
#define PNG_COLOR_TYPE_PALETTE 3
#endif

/* Forward decl of vuln func */

/* ENTRY: Neutralized pass-through — NO guards, NO checks */
