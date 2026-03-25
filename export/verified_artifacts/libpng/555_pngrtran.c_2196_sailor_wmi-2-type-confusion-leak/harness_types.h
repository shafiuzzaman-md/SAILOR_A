/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

// Minimal local type/macro definitions to compile the harness
#ifndef PNG_READ_STRIP_ALPHA_SUPPORTED
#define PNG_READ_STRIP_ALPHA_SUPPORTED 1
#endif

#ifndef PNG_STRIP_ALPHA
#define PNG_STRIP_ALPHA 0x1
#endif

#ifndef PNG_COLOR_MASK_ALPHA
#define PNG_COLOR_MASK_ALPHA 0x04
#endif

#ifndef PNG_COLOR_MASK_COLOR
#define PNG_COLOR_MASK_COLOR 0x02
#endif

#ifndef PNG_COLOR_TYPE_RGB
#define PNG_COLOR_TYPE_RGB 2
#endif

#ifndef PNG_COLOR_TYPE_GRAY
#define PNG_COLOR_TYPE_GRAY 0
#endif

#ifndef PNG_COLOR_TYPE_PALETTE
#define PNG_COLOR_TYPE_PALETTE 3
#endif

#ifndef PNG_COLOR_TYPE_RGB_ALPHA
#define PNG_COLOR_TYPE_RGB_ALPHA 6
#endif

#ifndef PNG_EXPAND
#define PNG_EXPAND 0x100
#endif

#ifndef PNG_EXPAND_tRNS
#define PNG_EXPAND_tRNS 0x200
#endif

#ifndef PNG_ADD_ALPHA
#define PNG_ADD_ALPHA 0x400
#endif

#ifndef PNG_FILLER
#define PNG_FILLER 0x800
#endif

typedef unsigned char png_byte;

typedef struct png_struct_s {
    unsigned int transformations;
    int num_trans;
    void *palette;
} png_struct;

typedef struct png_info_s {
    png_byte color_type;
    int bit_depth;
    int num_trans;
    int channels;
} png_info;

// Vulnerable logic isolated here. Entry wrapper calls this directly.
