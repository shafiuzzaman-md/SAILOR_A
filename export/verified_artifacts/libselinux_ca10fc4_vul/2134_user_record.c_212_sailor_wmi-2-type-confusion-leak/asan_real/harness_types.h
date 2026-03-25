/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>

// Minimal local type defs (from user_record.c preamble)
struct sepol_user {
    char *name;
    char *mls_level;
    char *mls_range;
    char **roles;
    unsigned int num_roles;
};

typedef struct sepol_user sepol_user_t;

// KEEP original function (vulnerable site at line with strcmp on roles[i])
