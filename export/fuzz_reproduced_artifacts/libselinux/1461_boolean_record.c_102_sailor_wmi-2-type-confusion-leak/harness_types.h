/* AUTO-GENERATED from harness preamble */
#pragma once


// harness/spine.c
#include <stdlib.h>
#include <string.h>

// Minimal local definitions to avoid pulling project headers
#ifndef STATUS_ERR
#define STATUS_ERR (-1)
#endif
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS (0)
#endif

// sepol types (minimal)
typedef void (*sepol_msg_callback_t)(void *arg, const char *msg);

typedef struct sepol_handle {
    sepol_msg_callback_t msg_callback;
    void *msg_callback_arg;
} sepol_handle_t;

struct sepol_bool {
    char *name;
    int value;
};
typedef struct sepol_bool sepol_bool_t;

// ERR macro approximating debug.h; unconditionally reads fields to materialize deref
#ifndef ERR
#define ERR(handle, fmt, ...) do { \
    sepol_handle_t *h_ = (handle); \
    if (h_) { \
        volatile sepol_msg_callback_t cb_ = h_->msg_callback; \
        volatile void *arg_ = h_->msg_callback_arg; \
        (void)cb_; (void)arg_; \
    } \
} while(0)
#endif

// Vulnerable function (verbatim vulnerable line inside)
int sepol_bool_set_name(sepol_handle_t * handle,
