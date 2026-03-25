/* AUTO-GENERATED from harness preamble */
#pragma once

/* minimal harness for png_set_pHYs vulnerability site */
#include <stdint.h>
#include <stdlib.h>

/* Minimal type aliases matching libpng */
typedef uint32_t png_uint_32;
typedef unsigned char png_byte;

typedef struct png_struct_def { int dummy; } png_struct;

typedef struct png_info_def {
    png_uint_32 x_pixels_per_unit;
    png_uint_32 y_pixels_per_unit;
    png_byte    phys_unit_type;
    png_uint_32 valid;
} png_info;

#ifndef PNG_INFO_pHYs
#define PNG_INFO_pHYs 0x0001u
#endif

#ifndef png_debug1
#define png_debug1(level, msg, arg)
#endif

/* Entry == Vulnerable function. Keep body and the exact vulnerable line; add sink after it. */
void
png_set_pHYs(const png_struct *png_ptr, png_info *info_ptr,
