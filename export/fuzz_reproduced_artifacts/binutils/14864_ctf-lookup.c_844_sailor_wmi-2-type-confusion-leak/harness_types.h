/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* Minimal local type defs to satisfy fields used on the path */
typedef unsigned long ctf_id_t;

typedef struct ctf_dict_s {
    int dummy; /* opaque for harness */
} ctf_dict_t;

typedef struct ctf_next_cu_s {
    ctf_dict_t *ctn_fp;
} ctf_next_cu_t;

typedef struct ctf_next_s {
    ctf_next_cu_t cu;
    void (*ctn_iter_fun)(void);
    size_t ctn_n;
} ctf_next_t;

/* Prototypes to match real signatures */

