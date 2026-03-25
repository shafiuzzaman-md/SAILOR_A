/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif

/* Minimal project types reconstructed for harness */
typedef struct sepol_context sepol_context_t;
typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_node {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
    sepol_context_t *con;
} sepol_node_t;

/* External helpers (stubbed in stubs.c) */

int sepol_node_set_addr(sepol_handle_t * handle,
