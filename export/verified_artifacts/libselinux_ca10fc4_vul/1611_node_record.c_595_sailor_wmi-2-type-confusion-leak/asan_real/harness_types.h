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

/* Minimal local types to avoid external headers */
typedef struct sepol_handle sepol_handle_t;  /* opaque */

typedef struct sepol_context sepol_context_t; /* opaque */

/* Struct must match field order for field 'con' */
typedef struct sepol_node {
    char *addr;        /* unused in sliced path */
    size_t addr_sz;    /* unused in sliced path */
    char *mask;        /* unused in sliced path */
    size_t mask_sz;    /* unused in sliced path */
    int proto;         /* unused in sliced path */
    sepol_context_t *con; /* used at vulnerable site */
} sepol_node_t;

/* External funcs we stub elsewhere */

