/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif

/* Minimal types needed on the path */
typedef void (*sepol_msg_callback_t)(void *arg, const char *fmt, ...);

typedef struct sepol_handle {
    sepol_msg_callback_t msg_callback;
    void *msg_callback_arg;
} sepol_handle_t;

typedef struct sepol_context {
    char *user;
    char *role;
    char *type;
    char *mls;
} sepol_context_t;

/* ERR macro modeled to dereference handle fields (matches real behavior) */
#ifndef ERR
#define ERR(h, fmt, ...) \
    do { \
        if ((h) && (h)->msg_callback) { \
            /* UAF manifests on reading these fields if h was freed */ \
            (h)->msg_callback((h)->msg_callback_arg, fmt, ##__VA_ARGS__); \
        } \
    } while (0)
#endif

/* Vulnerable function — keep exact line text for the vulnerable statement */
int sepol_context_set_mls(sepol_handle_t * handle,
