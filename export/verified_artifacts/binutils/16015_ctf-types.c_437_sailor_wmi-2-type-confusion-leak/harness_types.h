/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for ctf_type_iter -> ctf_type_next
 * Preserve the exact vulnerable statement from ctf-types.c:437
 */
#include <stddef.h>
#include <stdint.h>

#ifndef ECTF_NEXT_WRONGFUN
#define ECTF_NEXT_WRONGFUN 1001
#endif

/* Minimal type definitions to satisfy signatures and field accesses */
typedef int ctf_id_t; /* minimal */

typedef struct ctf_dict {
    int ctf_typemax;
    int ctf_flags;
} ctf_dict_t;

/* Function type: int (*)(ctf_id_t, void*) as used by libctf */
typedef int (ctf_type_f)(ctf_id_t, void *);

typedef struct ctf_next {
    int ctn_type;
    void (*ctn_iter_fun)(void);
    struct { ctf_dict_t *ctn_fp; } cu; /* present in real code, unused here */
} ctf_next_t;

/* Global seed for iterator, set by driver */
ctf_next_t *g_ctf_it_seed = NULL;

/* Forward decls */

/* ENTRY: pure pass-through to vulnerable function (no guards) */
