/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdarg.h>

// Minimal type models
typedef struct sepol_handle {
    void (*msg_callback)(void *arg, const char *fmt, ...);
    void *msg_callback_arg;
} sepol_handle_t;

typedef struct { int dummy; } policydb_t;

typedef struct {
    policydb_t p;
    int process_class;
} sepol_policydb_t;

typedef struct {
    int range;
} context_struct_t;

#ifndef STATUS_ERR
#define STATUS_ERR (-1)
#endif

// Emulate libsepol debug ERR macro that reads callback fields from handle
#ifndef ERR
#define ERR(h, fmt, ...) do { \
    if ((h) && (h)->msg_callback) { \
        (h)->msg_callback((h)->msg_callback_arg, fmt, ##__VA_ARGS__); \
    } \
} while (0)
#endif

// Vulnerable function (neutralized to force the target path)
int sepol_mls_check(sepol_handle_t * handle,
