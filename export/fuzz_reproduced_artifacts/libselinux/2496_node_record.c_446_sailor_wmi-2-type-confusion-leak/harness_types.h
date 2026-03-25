/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif

#ifndef SEPOL_PROTO_IP4
#define SEPOL_PROTO_IP4 4
#endif
#ifndef SEPOL_PROTO_IP6
#define SEPOL_PROTO_IP6 6
#endif

/* Minimal stand-ins for project types */
typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_node {
    char *addr; size_t addr_sz;
    char *mask; size_t mask_sz;
    int proto;
    void *con;
} sepol_node_t;

/* Silence error reporting in harness */
