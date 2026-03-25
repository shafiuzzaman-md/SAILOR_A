/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/* Minimal type aliases from libpng */
typedef unsigned char png_byte;
typedef size_t png_alloc_size_t;

/* Minimal png_struct with only the fields we access in the harness */
typedef struct png_struct_s {
    unsigned int transformed_pixel_depth;
    png_byte *row_buf;
    png_alloc_size_t width;
    unsigned int pass;
} png_struct;

/* Neutralized vulnerable function: keep signature + target case 2 body only */
