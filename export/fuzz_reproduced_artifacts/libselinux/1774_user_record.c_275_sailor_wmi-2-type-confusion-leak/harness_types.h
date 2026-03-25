/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif

// Minimal handle type (not used on the path, present for signature compatibility)
typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_user sepol_user_t;
struct sepol_user {
    char *name;
    char *mls_level;
    char *mls_range;
    char **roles;
    unsigned int num_roles;
};

// Keep the real free routine to materialize the UAF path
