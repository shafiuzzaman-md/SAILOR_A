/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c - minimal neutralized spine for ctf_enum_iter -> ctf_enum_next */
#include <stdint.h>
#include <stdlib.h>

/* Minimal project-local typedefs */
typedef int32_t ctf_id_t;
typedef struct ctf_dict {
    void *ctf_dictops; /* field intentionally touched by macros to model stale read */
} ctf_dict_t;

typedef struct ctf_enum {
    uint32_t cte_name;
    int32_t cte_value;
} ctf_enum_t;

typedef struct ctf_type {
    uint32_t ctt_info; /* info field used by macros */
} ctf_type_t;

typedef struct ctf_next {
    struct { ctf_dict_t *ctn_fp; } cu; /* not used on path but kept for shape */
    size_t ctn_increment;
    size_t ctn_n;
    void (*ctn_iter_fun)(void);
    union { const ctf_enum_t *ctn_en; void *ptr; } u;
} ctf_next_t;

typedef int (ctf_enum_f)(const char *name, int val, void *arg);

/* Macros: ensure the vulnerable statement text remains verbatim while touching fp */
#ifndef LCTF_INFO_VLEN
/* Touch fp->ctf_dictops (stale read if fp is freed), then compute some vlen from info */
#define LCTF_INFO_VLEN(fp, info) ((void)((fp)->ctf_dictops), ((uint32_t)(info) & 0xffu))
#endif

