/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "harness_types.h"

// Minimal type definitions if not provided by harness_types.h
#ifndef SAILOR_HAVE_SYMTAB_USER_TYPES
#define SAILOR_HAVE_SYMTAB_USER_TYPES

typedef struct symtab_datum {
    uint32_t value;
} symtab_datum_t;

typedef struct user_datum {
    symtab_datum_t s;
} user_datum_t;

#endif

