/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/pngwrite.c - neutralized spine for png_write_png -> png_write_info with WMI-2 UAF modeling */
#ifndef PNG_WRITE_TEXT_SUPPORTED
#define PNG_WRITE_TEXT_SUPPORTED 1
#endif
#include <stdint.h>
#include <stdlib.h>

/* Minimal local type defs sufficient for this harness */
typedef struct png_struct_def { int dummy; } png_struct;
typedef struct png_info_def {
    int num_text; /* vulnerable field read */
} png_info;

/* Forward decl */

/* ENTRY: MUST be a direct pass-through, no guards */
