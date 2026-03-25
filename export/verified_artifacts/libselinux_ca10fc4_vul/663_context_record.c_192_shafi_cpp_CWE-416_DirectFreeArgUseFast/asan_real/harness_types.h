/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>

#ifndef KLEE_ASSUME
#define KLEE_ASSUME(x) klee_assume(x)
#endif

// Minimal local type definition (from preamble excerpt)
typedef struct sepol_context {
    char *user;
    char *role;
    char *type;
    char *mls;
} sepol_context_t;

// Vulnerable function (neutralized, keep exact vulnerable statement)
