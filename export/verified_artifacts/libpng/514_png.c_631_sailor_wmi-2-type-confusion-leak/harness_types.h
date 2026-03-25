/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for libpng: png_destroy_info_struct -> png_free_data */
#include <stdlib.h>
#include <string.h>

/* Local defines to ensure the vulnerable block is compiled */
#ifndef PNG_eXIf_SUPPORTED
#define PNG_eXIf_SUPPORTED 1
#endif

#ifndef PNG_FREE_EXIF
#define PNG_FREE_EXIF 0x00000001u
#endif
#ifndef PNG_INFO_eXIf
#define PNG_INFO_eXIf 0x00000001u
#endif
#ifndef PNG_FREE_ALL
#define PNG_FREE_ALL 0xFFFFFFFFu
#endif

/* Minimal typedefs to match signatures */
typedef unsigned int png_uint_32;

typedef struct png_struct_def {
    int dummy;
} png_struct;

typedef struct png_info_def {
    png_uint_32 free_me;
    png_uint_32 valid;
    void *exif;
} png_info;

/* Stubs referenced from path (also provided in stubs.c, duplicate-safe if inline). */

/* ENTRY: must be a direct pass-through (no guards, no checks) */
