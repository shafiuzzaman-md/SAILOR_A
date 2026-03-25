/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness spine for png_write_PLTE vulnerability reach */
#include <stddef.h>
#include <stdint.h>

/* Minimal libpng-like typedefs/macros to satisfy compilation */
typedef uint32_t png_uint_32;
typedef uint16_t png_uint_16;
typedef uint8_t  png_byte;
typedef png_byte* png_bytep;

typedef struct png_struct {
    png_byte color_type;
    png_byte bit_depth;
    png_uint_16 num_palette;
    png_uint_32 mode;
    png_uint_32 width, height;
    png_byte channels;
    png_byte do_filter;
    unsigned int mng_features_permitted;
} png_struct;

typedef struct png_color {
    png_byte red;
    png_byte green;
    png_byte blue;
} png_color;

#ifndef PNG_COLOR_TYPE_PALETTE
#define PNG_COLOR_TYPE_PALETTE 3
#endif
#ifndef PNG_COLOR_MASK_COLOR
#define PNG_COLOR_MASK_COLOR 2
#endif
#ifndef PNG_MAX_PALETTE_LENGTH
#define PNG_MAX_PALETTE_LENGTH 256
#endif
#ifndef PNG_HAVE_PLTE
#define PNG_HAVE_PLTE 0x0100
#endif
#ifndef PNG_POINTER_INDEXING_SUPPORTED
#define PNG_POINTER_INDEXING_SUPPORTED 1
#endif

/* Chunk name constant */
static png_byte png_PLTE[4] = { 'P','L','T','E' };

/* Externs to be provided by stubs.c */

/* Neutralized vulnerable function: keep the vulnerable statements verbatim */
void png_write_PLTE(png_struct *png_ptr, const png_color *palette, png_uint_32 num_pal);
