/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for MuPDF path clone vulnerability
 * Spine: fz_clone_path -> clone_block
 */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef FZ_PATH_UNPACKED
#define FZ_PATH_UNPACKED 0
#endif

/* Minimal type definitions needed by driver and entry */
typedef struct fz_context { int dummy; } fz_context;

typedef struct fz_path {
    int packed;
    int cmd_len;
    int cmd_cap;
    unsigned char *cmds;
    int coord_len;
    int coord_cap;
    float *coords;
    struct { float x, y; } current, begin;
} fz_path;

