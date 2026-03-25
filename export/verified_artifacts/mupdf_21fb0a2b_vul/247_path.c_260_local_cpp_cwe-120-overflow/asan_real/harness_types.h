/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Minimal local type definitions needed by fz_pack_path

typedef struct { float x, y; } fz_point;

typedef struct fz_context_s { int dummy; } fz_context; // opaque

struct fz_path
{
    int8_t refs;
    uint8_t packed;
    int cmd_len, cmd_cap;
    unsigned char *cmds;
    int coord_len, coord_cap;
    float *coords;
    fz_point current;
    fz_point begin;
};

typedef struct
{
    int8_t refs;
    uint8_t packed;
    uint8_t coord_len;
    uint8_t cmd_len;
} fz_packed_path;

enum
{
    FZ_PATH_UNPACKED = 0,
    FZ_PATH_PACKED_FLAT = 1,
    FZ_PATH_PACKED_OPEN = 2
};

// Vulnerable function (neutralized to the target path). Keep vulnerable statements verbatim.
