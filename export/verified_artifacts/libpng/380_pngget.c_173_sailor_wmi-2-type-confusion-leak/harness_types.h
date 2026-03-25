/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

// Minimal local libpng-type definitions for harness
#ifndef PNG_TYPES
#define PNG_TYPES

typedef unsigned int png_uint_32;
typedef unsigned char png_byte;

typedef struct png_struct_def {
    int dummy;  // placeholder; png_get_pixels_per_meter only checks non-NULL
} png_struct;

typedef struct png_info_def {
    png_uint_32 valid;
    png_uint_32 x_pixels_per_unit;
    png_uint_32 y_pixels_per_unit;
    png_byte    phys_unit_type;
} png_info;

#endif

// Define required macros to keep the target path compiled-in
#ifndef PNG_pHYs_SUPPORTED
#define PNG_pHYs_SUPPORTED 1
#endif
#ifndef PNG_INFO_pHYs
#define PNG_INFO_pHYs 0x0001u
#endif
#ifndef PNG_RESOLUTION_METER
#define PNG_RESOLUTION_METER 1
#endif
#ifndef PNG_UNUSED
#define PNG_UNUSED(x) (void)(x)
#endif
