/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>

// Minimal local type defs based on preamble
struct sepol_user {
    char *name;
    char *mls_level;
    char *mls_range;
    char **roles;
    unsigned int num_roles;
};

struct sepol_user_key {
    char *name;
};

typedef struct sepol_user sepol_user_t;
typedef struct sepol_user_key sepol_user_key_t;

// Real free function from source (must actually free)
