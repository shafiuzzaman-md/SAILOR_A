/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// Minimal local types to compile the slice
typedef struct sepol_handle {
    void (*msg_callback)(void *, const char *);
    void *msg_callback_arg;
} sepol_handle_t;

typedef struct { int dummy; } sepol_policydb_t;
typedef struct { int dummy; } sepol_user_key_t;
typedef struct { int dummy; } sepol_user_t;
typedef struct { int dummy; } role_datum_t;

enum { STATUS_SUCCESS = 0, STATUS_ERR = -1 };

// ERR macro models the vulnerable read via handle->msg_callback and ->msg_callback_arg
#ifndef ERR
#define ERR(handle, fmt, ...) do { \
    sepol_handle_t *_h = (handle); \
    /* Read through stale pointer fields (type-confusion/UAF read) */ \
    (void)(_h ? _h->msg_callback : (void*)0); \
    if (_h && _h->msg_callback) { \
        char _buf[256]; \
        snprintf(_buf, sizeof(_buf), fmt, __VA_ARGS__); \
        _h->msg_callback(_h->msg_callback_arg, _buf); \
    } else { \
        /* still touch msg_callback_arg to model read */ \
        (void)(_h ? _h->msg_callback_arg : (void*)0); \
    } \
} while(0)
#endif

// Vulnerable function (neutralized to the target site only)
int sepol_user_modify(sepol_handle_t * handle,
                      sepol_policydb_t * p,
