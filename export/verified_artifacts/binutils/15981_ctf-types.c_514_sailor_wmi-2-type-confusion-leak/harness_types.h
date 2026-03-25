/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for ctf-variable iteration path */
#include <stdlib.h>
#include <stdint.h>

#ifndef ENOMEM
#define ENOMEM 12
#endif
#ifndef ECTF_NEXT_WRONGFUN
#define ECTF_NEXT_WRONGFUN 1001
#endif
#ifndef ECTF_NEXT_WRONGFP
#define ECTF_NEXT_WRONGFP 1002
#endif
#ifndef ECTF_NEXT_END
#define ECTF_NEXT_END 2000
#endif
#ifndef LCTF_CHILD
#define LCTF_CHILD 0x1
#endif
#ifndef CTF_ERR
#define CTF_ERR (-1)
#endif

/* Forward decls */
typedef int ctf_id_t;

typedef struct ctf_dvdef {
    const char *dvd_name;
    ctf_id_t dvd_type;
    struct ctf_dvdef *next;
} ctf_dvdef_t;

typedef struct ctf_varent {
    int ctv_name;
    ctf_id_t ctv_type;
} ctf_varent_t;

typedef struct ctf_dict {
    int ctf_flags;
    struct ctf_dict *ctf_parent;
    int ctf_nvars;
    ctf_varent_t *ctf_vars;
    ctf_dvdef_t *ctf_dvdefs;
} ctf_dict_t;

typedef struct ctf_next {
    struct { ctf_dict_t *ctn_fp; } cu;
    void (*ctn_iter_fun)(void);
    union { ctf_dvdef_t *ctn_dvd; } u;
    unsigned long ctn_n;
} ctf_next_t;

typedef int (ctf_variable_f)(const char *name, ctf_id_t type, void *arg);

/* Stubs (implemented in stubs.c but declare here) */

/* ENTRY: neutralized pass-through to vulnerable function (MANDATORY PATTERN) */
