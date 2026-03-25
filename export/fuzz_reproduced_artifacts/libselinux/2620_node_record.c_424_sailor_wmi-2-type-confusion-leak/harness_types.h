/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif

// Minimal handle and node types sufficient for this path
typedef void (*sepol_msg_callback_t)(void *, const char *, ...);

typedef struct sepol_handle {
    sepol_msg_callback_t msg_callback;
    void *msg_callback_arg;
} sepol_handle_t;

typedef struct sepol_node {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
    void *con;
} sepol_node_t;

#ifndef ERR
#define ERR(handle, fmt, ...) do { \
    if ((handle) && (handle)->msg_callback) \
        (handle)->msg_callback((handle)->msg_callback_arg, fmt, ##__VA_ARGS__); \
} while (0)
#endif

// Vulnerable function (neutralized, keep exact vulnerable statement)
int sepol_node_set_addr_bytes(sepol_handle_t * handle,
                              sepol_node_t * node,
