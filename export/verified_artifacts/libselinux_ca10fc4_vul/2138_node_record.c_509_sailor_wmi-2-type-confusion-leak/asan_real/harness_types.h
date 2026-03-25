/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif

// Minimal sepol_handle_t capturing fields used by ERR
typedef struct sepol_handle {
    void (*msg_callback)(void *arg, const char *fmt, ...);
    void *msg_callback_arg;
    int msg_level; // unused but common in real headers
} sepol_handle_t;

// Minimal sepol_node_t as seen in the preamble excerpt
typedef struct sepol_node {
    char *addr; size_t addr_sz;
    char *mask; size_t mask_sz;
    int proto;
    void *con;
} sepol_node_t;

// Recreate ERR macro behavior to dereference handle fields (msg_callback, arg)
#ifndef ERR
#define ERR(h, fmt, ...) \
    do { \
        if ((h) && (h)->msg_callback) \
            (h)->msg_callback((h)->msg_callback_arg, fmt, ##__VA_ARGS__); \
    } while (0)
#endif

// Vulnerable function — keep signature and the vulnerable statement verbatim
int sepol_node_set_mask_bytes(sepol_handle_t * handle,
			      sepol_node_t * node,
