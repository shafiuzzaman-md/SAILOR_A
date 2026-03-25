/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

// Minimal local type defs to compile standalone
#ifndef XML_HIDDEN
#define XML_HIDDEN /* empty */
#endif
struct _xmlDict { int seed; int size; void *table; void *subdict; int limit; };

// Project-minimal typedefs
typedef struct sepol_handle sepol_handle_t;  // opaque
typedef struct sepol_user sepol_user_t;      // opaque

typedef struct { unsigned int value; } symtab_datum_t;

typedef struct user_datum {
    symtab_datum_t s;  // has 'value'
    // other fields omitted
} user_datum_t;

typedef struct {
    struct { unsigned int nprim; void *table; } p_users;
    char **p_user_val_to_name;       // not used on our path
    user_datum_t **user_val_to_struct; // reverse map array
} policydb_t;

