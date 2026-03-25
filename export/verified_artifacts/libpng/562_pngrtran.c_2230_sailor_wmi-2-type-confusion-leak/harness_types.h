/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// Minimal local type defs sufficient for the vulnerable statements
typedef unsigned char png_byte;
typedef unsigned int png_uint_32;

typedef struct png_struct_def {
    png_uint_32 info_rowbytes;
    unsigned int transformations;
    unsigned int num_trans;
    unsigned char user_transform_depth;
    unsigned char user_transform_channels;
    void *palette;
} png_struct;

typedef struct png_info_def {
    png_byte color_type;
    png_byte bit_depth;
    png_byte channels;
    png_byte pixel_depth;
    png_uint_32 width;
    size_t rowbytes;
} png_info;

#ifndef PNG_ROWBYTES
// Standard libpng computation: ((pixel_depth + 7) >> 3) * width
#define PNG_ROWBYTES(pixel_depth, width) ((((pixel_depth) + 7) >> 3) * (width))
#endif

// Entry == Vulnerable function: keep only the vulnerable statements and sink.
