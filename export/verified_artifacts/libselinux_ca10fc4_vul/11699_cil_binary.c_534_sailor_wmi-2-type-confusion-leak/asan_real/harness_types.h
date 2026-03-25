/* AUTO-GENERATED from harness preamble */
#pragma once

/* spine.c — minimal sliced harness for cil_binary.c:534 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef SEPOL_OK
#define SEPOL_OK 0
#endif
#ifndef SEPOL_ERR
#define SEPOL_ERR (-1)
#endif
#ifndef TYPE_TYPE
#define TYPE_TYPE 1
#endif

/* Minimal project-specific types to satisfy signatures */
typedef struct type_datum_s {
    struct { uint32_t value; } s;  /* matches usage: sepol_*->s.value */
    int primary;
    int flavor;
} type_datum_t;

struct cil_symtab_datum { int dummy; };

struct cil_alias_datum { char *fqn; };

struct cil_alias {
    struct cil_alias_datum datum;    /* used for datum.fqn in original code (not on our path) */
    struct cil_symtab_datum *actual; /* used via DATUM(cil_alias->actual) */
};

/* Provide a tiny policydb_t with room for stub cooperation if needed */
typedef struct policydb_s {
    void *opaque;
} policydb_t;

#ifndef DATUM
#define DATUM(x) ((struct cil_symtab_datum *)(x))
#endif

/* Forward decl for stub provided in stubs.c */

/* VULNERABLE FUNCTION — neutralized slice keeping the vulnerable statement verbatim */
