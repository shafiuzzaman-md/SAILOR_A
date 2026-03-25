/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal neutralized harness for libpng png_write_reinit */
#include <stdint.h>
#include <stdlib.h>

/* Minimal local types to satisfy signatures */
typedef uint32_t png_uint_32;

typedef struct png_struct {
    int dummy;
} png_struct;

typedef struct png_info {
    int bit_depth;
    int color_type;
    int interlace_type;
    int compression_type;
    int filter_type;
} png_info;

/* External function to be stubbed */
void png_set_IHDR(png_struct *png_ptr, png_info *info_ptr, png_uint_32 width,
                  png_uint_32 height, int bit_depth, int color_type,
                  int interlace_type, int compression_type, int filter_type);

/* Entry == vulnerable function: keep only the vulnerable statement and sink */
void /* PRIVATE */
png_write_reinit(png_struct *png_ptr, png_info *info_ptr,
