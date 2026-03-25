/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Minimal local type stubs to satisfy role_datum_init signature
#ifndef EBMAP_DEFINED
#define EBMAP_DEFINED
typedef struct { int dummy; } ebitmap_t;
#endif

#ifndef TYPE_SET_DEFINED
#define TYPE_SET_DEFINED
typedef struct { int dummy; } type_set_t;
#endif

#ifndef ROLE_DATUM_DEFINED
#define ROLE_DATUM_DEFINED
typedef struct role_datum {
    ebitmap_t dominates;
    type_set_t types;   // field accessed at the vulnerable site
    ebitmap_t cache;
    ebitmap_t roles;
} role_datum_t;
#endif

// Minimal API stubs referenced around the target site

// Vulnerable function neutralized to the exact sink line
// Original sink line (policydb.c:591):
//     type_set_init(&x->types);
