/* AUTO-GENERATED from harness preamble */
#pragma once

/* spine_harness.c - neutralized harness for png_set_keep_unknown_chunks */
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#ifndef PNG_HANDLE_CHUNK_LAST
#define PNG_HANDLE_CHUNK_LAST 8
#endif

typedef unsigned char png_byte;

typedef struct png_struct {
    png_byte *chunk_list;
    unsigned int num_chunk_list;
    int unknown_default;
} png_struct;

/* Neutralized version: keep only the vulnerable region */
void
png_set_keep_unknown_chunks(png_struct *png_ptr, int keep,
