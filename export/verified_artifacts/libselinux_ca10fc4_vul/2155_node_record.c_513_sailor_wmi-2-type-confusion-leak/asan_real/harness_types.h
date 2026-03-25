/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

// Local minimal type defs (avoid pulling full project headers)
typedef struct sepol_handle sepol_handle_t;  // opaque for logging only
struct sepol_handle { int dummy; };

typedef struct sepol_context sepol_context_t; // opaque, unused here

// Reconstruct sepol_node_t layout used by target function
typedef struct sepol_node {
    char *addr;      size_t addr_sz;
    char *mask;      size_t mask_sz;
    int proto;
    sepol_context_t *con;
} sepol_node_t;

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR (-1)
#endif
