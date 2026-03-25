/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for libpng: png_destroy_info_struct -> png_free_data */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Local macro and type definitions (minimal) */
#ifndef PNG_tRNS_SUPPORTED
#define PNG_tRNS_SUPPORTED 1
#endif
#ifndef PNG_FREE_TRNS
#define PNG_FREE_TRNS 0x0010u
#endif
#ifndef PNG_FREE_ALL
#define PNG_FREE_ALL 0xFFFFu
#endif
#ifndef PNG_INFO_tRNS
#define PNG_INFO_tRNS 0x0001u
#endif

/* Minimal typedefs to satisfy signatures */
typedef uint32_t png_uint_32;
typedef unsigned char png_byte;

typedef struct png_struct_def {
    int dummy;
} png_struct;

typedef struct png_info_def {
    png_uint_32 free_me;
    png_uint_32 valid;
    png_byte *trans_alpha;
    int num_trans;
} png_info;

/* Decls for external helpers we stub elsewhere */

/* ENTRY: neutralized pass-through (no guards, no early returns) */
