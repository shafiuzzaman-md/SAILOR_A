/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef OPJ_COMMON_CBLK_DATA_EXTRA
#define OPJ_COMMON_CBLK_DATA_EXTRA 8
#endif

typedef unsigned char OPJ_BYTE;
typedef unsigned int OPJ_UINT32;

typedef struct opj_mqc {
    OPJ_BYTE *backup;
    OPJ_BYTE *end;
} opj_mqc_t;

/* Vulnerable function (as in source excerpt) */
