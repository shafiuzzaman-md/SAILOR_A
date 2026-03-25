/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for cil_type_to_policydb focusing on line 474 sink */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Opaque/forward declarations to avoid heavy includes
typedef struct policydb policydb_t;

struct cil_symtab_datum { 
    char *fqn; 
};

struct cil_type {
    struct cil_symtab_datum datum;
    struct cil_type *bounds;
    int value;
};

