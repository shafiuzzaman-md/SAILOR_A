/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifndef SEPOL_OK
#define SEPOL_OK 0
#endif
#ifndef SEPOL_ERR
#define SEPOL_ERR -1
#endif

// Minimal stand-ins for external types used only as opaque pointers
typedef struct policydb { int dummy; } policydb_t;

typedef struct cil_symtab_datum { void *dummy; char *fqn; } cil_symtab_datum_t; // not used here, kept minimal

struct cil_typeattribute {
    struct cil_symtab_datum datum;
    struct cil_list *expr_list;
    ebitmap_t *types;
    int used;    // whether or not this attribute was used in a binary policy rule
    int keep;
};

// Forward declare opaque types referenced by struct but unused in our slice
struct cil_list; typedef struct { int unused; } ebitmap_t;

