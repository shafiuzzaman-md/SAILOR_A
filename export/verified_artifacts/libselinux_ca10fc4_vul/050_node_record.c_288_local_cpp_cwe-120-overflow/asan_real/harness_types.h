/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif

/* Minimal stand-ins for project types */
typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_node {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
    void *con;
} sepol_node_t;

typedef struct sepol_node_key {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
} sepol_node_key_t;

/* Debug macro used by original code */
