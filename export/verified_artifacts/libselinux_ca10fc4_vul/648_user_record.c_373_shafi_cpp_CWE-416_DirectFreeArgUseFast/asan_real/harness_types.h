/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>

// Minimal type needed for sepol_user_free
typedef struct sepol_user {
    char *name;
    unsigned int num_roles;
    char **roles;
    void *mls_level;
    void *mls_range;
} sepol_user_t;

// Vulnerable function (neutralized minimal copy with the exact vulnerable statement)
