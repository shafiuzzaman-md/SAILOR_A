/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c - minimal neutralized harness for libpng write path */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef PNG_WRITE_sRGB_SUPPORTED
#define PNG_WRITE_sRGB_SUPPORTED 1
#endif

/* Minimal type universe to compile the harness */
typedef uint32_t png_uint_32;
typedef unsigned char png_byte;

/* Opaque/minimal structs: only fields used on the path are modeled */
typedef struct png_struct_def {
    int dummy; /* not used in this harness */
} png_struct;

struct png_info_def {
    png_byte rendering_intent; /* vulnerable read uses this */
};
typedef struct png_info_def png_info;

/* External on path: provide as stub in stubs.c */

/* VULNERABLE FUNCTION (neutralized to the minimal path) */
