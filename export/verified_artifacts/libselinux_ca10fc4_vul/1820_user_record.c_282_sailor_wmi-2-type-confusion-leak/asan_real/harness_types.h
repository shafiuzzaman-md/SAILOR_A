/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif

// Minimal stand-in types needed for the path
typedef struct sepol_handle {
    void *msg_callback;     // accessed indirectly by ERR path
    void *msg_callback_arg; // accessed indirectly by ERR path
} sepol_handle_t;

typedef struct sepol_user {
    char *name;
    char *mls_level;
    char *mls_range;
    char **roles;
    unsigned int num_roles;
} sepol_user_t;

