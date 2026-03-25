/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef EINVAL
#define EINVAL 22
#endif
#ifndef ECTF_DMODEL
#define ECTF_DMODEL 1001
#endif
#ifndef LCTF_CHILD
#define LCTF_CHILD 0x1
#endif

/* Minimal struct matching fields used by ctf_import */
typedef struct ctf_dict {
    int ctf_refcnt;
    int ctf_dmodel;
    struct ctf_dict *ctf_parent;
    int ctf_parent_unreffed;
    void *ctf_pptrtab;
    unsigned long ctf_pptrtab_len;
    unsigned long ctf_pptrtab_typemax;
    char *ctf_parname;
    unsigned int ctf_flags;
} ctf_dict_t;

/* Declarations for externs referenced by ctf_import */

/* Vulnerable function: keep the vulnerable statement verbatim, then universal sink. */
int
