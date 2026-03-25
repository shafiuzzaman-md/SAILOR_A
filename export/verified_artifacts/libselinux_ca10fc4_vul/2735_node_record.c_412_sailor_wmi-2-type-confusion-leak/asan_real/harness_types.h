/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

// Minimal type definitions needed by the harness
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR (-1)
#endif

// Minimal sepol_handle_t with fields used by ERR macro path
typedef void (*sepol_msg_callback_t)(void *arg, const char *fmt, ...);
typedef struct sepol_handle {
    sepol_msg_callback_t msg_callback;
    void *msg_callback_arg;
} sepol_handle_t;

// Minimal sepol_node_t with only fields used by sepol_node_set_addr
typedef struct sepol_node {
    char *addr;
    size_t addr_sz;
    int proto;
} sepol_node_t;

// Define ERR exactly as used at the vulnerable site, and ensure it dereferences
// the (potentially freed) handle->msg_callback field to trigger UAF on read.
#define ERR(h, fmt, ...) do { \
    /* This read of (h)->msg_callback is the stale dereference KLEE should flag */ \
    if ((h) && (h)->msg_callback) { \
        /* We do not actually call the callback to avoid extra dependencies */ \
        /* (h)->msg_callback((h)->msg_callback_arg, fmt, ##__VA_ARGS__); */ \
    } \
} while (0)

// Vulnerable function (from node_record.c)
int sepol_node_set_addr(sepol_handle_t * handle,
