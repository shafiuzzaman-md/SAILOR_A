/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif

/* Minimal local types to match project usage */
typedef struct sepol_handle sepol_handle_t;
typedef struct sepol_node sepol_node_t;

struct sepol_handle {
    void (*msg_callback)(void *arg, const char *fmt, ...);
    void *msg_callback_arg;
    int msg_level;
};

struct sepol_node {
    char *addr;
    size_t addr_sz;
    char *mask;
    size_t mask_sz;
    int proto;
    void *con;
};

/* Declare a controllable malloc shim just for this TU */
#define malloc(sz) klee_malloc_fail(sz)

/* ERR macro: keep vulnerable statement text identical in call sites */
#define ERR(handle, fmt, ...)                                                      \
    do {                                                                           \
        sepol_handle_t *_h = (handle);                                             \
        if (_h && _h->msg_callback)                                                \
            _h->msg_callback(_h->msg_callback_arg, fmt, ##__VA_ARGS__);            \
    } while (0)

/* === Vulnerable function (neutralized, exact sink preserved) === */
int sepol_node_get_mask_bytes(sepol_handle_t * handle,
			      const sepol_node_t * node,
