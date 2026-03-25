/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for png_get_pixel_aspect_ratio_fixed */
#include <stddef.h>

/* Enable code paths used by the target function */
#ifndef PNG_READ_pHYs_SUPPORTED
#define PNG_READ_pHYs_SUPPORTED
#endif
#ifndef PNG_FIXED_POINT_SUPPORTED
#define PNG_FIXED_POINT_SUPPORTED
#endif

/* Minimal typedefs to satisfy the target function */
typedef unsigned int png_uint_32;
typedef int png_int_32;
typedef int png_fixed_point;
typedef unsigned char png_byte;

typedef struct png_struct_def { int _dummy; } png_struct; /* only non-NULL check */

typedef struct png_info_def {
    png_uint_32 valid;
    png_uint_32 x_pixels_per_unit;
    png_uint_32 y_pixels_per_unit;
    png_byte    phys_unit_type; /* not used here, but common in related getters */
} png_info;

/* Required macros (reasonable local defaults) */
#ifndef PNG_INFO_pHYs
#define PNG_INFO_pHYs 0x1
#endif
#ifndef PNG_UINT_31_MAX
#define PNG_UINT_31_MAX 0x7FFFFFFF
#endif
#ifndef PNG_FP_1
#define PNG_FP_1 100000
#endif

