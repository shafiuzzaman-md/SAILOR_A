/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdlib.h>

// Minimal type definitions (local, project-agnostic)
typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_user {
    char *name;
    char *mls_level;
    char *mls_range;
    char **roles;
    unsigned int num_roles;
} sepol_user_t;

// External API on path (stubbed separately)

// Vulnerable function (neutralized) — keep the exact vulnerable statement
int sepol_user_clone(sepol_handle_t * handle,
