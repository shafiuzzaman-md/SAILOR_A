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

/* Minimal stand-ins to avoid pulling project headers */
typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_context sepol_context_t; /* opaque */

struct sepol_port {
    /* Low - High range. Same for single ports. */
    int low, high;
    /* Protocol */
    int proto;
    /* Context */
    sepol_context_t *con;
};

typedef struct sepol_port sepol_port_t;

struct sepol_port_key {
    /* Low - High range. Same for single ports. */
    int low, high;
    /* Protocol */
    int proto;
};

typedef struct sepol_port_key sepol_port_key_t;

