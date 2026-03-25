/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness spine for ctf-serialize.c path: ctf_gzwrite -> symtypetab_density */
#include <stddef.h>
#include <stdlib.h>

/* Minimal type and macro definitions needed by the harness */
typedef struct { int _dummy; } ctf_dict_t;
typedef struct { int _dummy; } ctf_dynhash_t;
typedef void* gzFile;

typedef struct ctf_link_sym {
    int st_type;
    int st_nameidx_set;
} ctf_link_sym_t;

#ifndef CTF_SYMTYPETAB_EMIT_FUNCTION
#define CTF_SYMTYPETAB_EMIT_FUNCTION 0x1
#endif
#ifndef CTF_SYMTYPETAB_EMIT_PAD
#define CTF_SYMTYPETAB_EMIT_PAD 0x2
#endif
#ifndef CTF_SYMTYPETAB_FORCE_INDEXED
#define CTF_SYMTYPETAB_FORCE_INDEXED 0x4
#endif
#ifndef STT_FUNC
#define STT_FUNC 2
#endif
#ifndef STT_OBJECT
#define STT_OBJECT 1
#endif

/* Vulnerable function (neutralized) — keep signature and the exact vulnerable statement */
static int
symtypetab_density (ctf_dict_t *fp, ctf_dict_t *symfp, ctf_dynhash_t *symhash,
                    size_t *count, size_t *max, size_t *unpadsize,
