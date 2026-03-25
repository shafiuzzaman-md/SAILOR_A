/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifndef PNG_FREE_PCAL
#define PNG_FREE_PCAL (1U<<5)
#endif

typedef int32_t png_int_32;
typedef unsigned char png_byte;

typedef struct png_struct_def {
    int dummy;
} png_struct;

typedef struct png_info_def {
    unsigned long free_me;
} png_info;

/* Entry == Vulnerable function. Keep signature, keep ONLY the vulnerable statement verbatim, then assert. */
void png_set_pCAL(const png_struct *png_ptr, png_info *info_ptr,
    const char *purpose, png_int_32 X0, png_int_32 X1, int type,
