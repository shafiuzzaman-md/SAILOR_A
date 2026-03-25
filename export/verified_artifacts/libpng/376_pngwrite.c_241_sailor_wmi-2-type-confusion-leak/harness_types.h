/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>

/* Minimal local typedefs/macros to compile the slice */
typedef uint32_t png_uint_32;
typedef uint8_t  png_byte;

typedef struct png_struct_def {
    int dummy;
} png_struct;

typedef struct png_color_struct { png_byte red, green, blue; } png_color;
typedef png_color* png_colorp;

typedef struct png_sig_bit_struct {
    png_byte red, green, blue, alpha;
} png_sig_bit;

typedef struct png_info_def {
    png_uint_32 valid;
    int color_type;
    png_colorp palette;
    unsigned int num_palette;
    unsigned int num_trans;
    png_sig_bit sig_bit;
} png_info;

#ifndef PNG_INFO_PLTE
#define PNG_INFO_PLTE 0x0008u
#endif
#ifndef PNG_COLOR_TYPE_PALETTE
#define PNG_COLOR_TYPE_PALETTE 3
#endif

/* Externs for stubbed helpers */

/* ENTRY: strict pass-through to vulnerable function (no guards!) */
