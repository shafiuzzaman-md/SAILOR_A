/* AUTO-GENERATED from harness preamble */
#pragma once

/* minimal sliced harness for imgutils.c:39 */
#include <stdint.h>
#include <string.h>

/* Minimal type shims (only fields used on the path) */
typedef struct AVComponentDescriptor {
    int plane;  /* which of the 4 planes */
    int step;   /* elements between consecutive pixels */
} AVComponentDescriptor;

typedef struct AVPixFmtDescriptor {
    const char *name;
    uint8_t nb_components;
    uint8_t log2_chroma_w;
    uint8_t log2_chroma_h;
    uint64_t flags;
    AVComponentDescriptor comp[4];
} AVPixFmtDescriptor;

/* Vulnerable function — keep exact vulnerable statements, neutralize the rest */
void av_image_fill_max_pixsteps(int max_pixsteps[4], int max_pixstep_comps[4],
