/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c - minimal sliced harness */
#include <stddef.h>
#include <stdint.h>

/* Minimal local typedefs to satisfy signatures */
typedef struct ctf_dict_t {
    void *ctf_objthash; /* only field used at target site */
} ctf_dict_t;

typedef struct emit_symtypetab_state_t {
    struct ctf_dict_t *symfp; /* used as 2nd arg */
    size_t maxobjt;           /* used as 5th arg (by address) */
    int symflags;             /* used as 9th arg */
} emit_symtypetab_state_t;

typedef struct ctf_header_t { int _dummy; } ctf_header_t;

typedef void* gzFile; /* for entry signature */

/* External function called at the vulnerable site (auto-stubbed) */
int symtypetab_density(ctf_dict_t *fp, ctf_dict_t *symfp, void *chash,
                       size_t *nsyms, size_t *maxsym, size_t *unpadsize,
                       size_t *padsize, size_t *idx_size, int flags);

/* VULNERABLE FUNCTION (neutralized, keep only the target statement) */
static int
ctf_symtypetab_sect_sizes (ctf_dict_t *fp, emit_symtypetab_state_t *s,
                           ctf_header_t *hdr, size_t *objt_size,
                           size_t *func_size, size_t *objtidx_size,
                           size_t *funcidx_size)
{
    size_t nfuncs, nobjts;
    size_t objt_unpadsize, func_unpadsize, objt_padsize, func_padsize;

    /* === VULNERABLE STATEMENT (verbatim) === */
    if (symtypetab_density (fp, s->symfp, fp->ctf_objthash, &nobjts, &s->maxobjt,
			  &objt_unpadsize, &objt_padsize, objtidx_size,
			  s->symflags) < 0)
        return -1;					/* errno is set for us.  */

    /* Universal sink assertion to mark reachability if no crash occurred */
    return 0;
}

/* ENTRY FUNCTION: direct pass-through to vulnerable function (no guards) */
int
