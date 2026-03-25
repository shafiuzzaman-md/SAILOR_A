/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

// Minimal real-layout approximation needed for ERR expansion
// Keep only fields used by the vulnerable macro path.
typedef struct sepol_handle_t {
    // Function pointer invoked by ERR()
    int (*msg_callback)(void *arg, int level, const char *fmt, ...);
    void *msg_callback_arg;
} sepol_handle_t;

typedef struct { int dummy; } sepol_policydb_t;  // unused in our slice
typedef struct { int dummy; } sepol_user_key_t;   // unused in our slice
typedef struct { int dummy; } sepol_user_t;       // unused in our slice

#ifndef SEPOL_MSG_ERR
#define SEPOL_MSG_ERR 3
#endif

// Define ERR to dereference handle and call through the callback like the real code
#ifndef ERR
#define ERR(handle, fmt, ...) do { \
    if ((handle) && (handle)->msg_callback) \
        (handle)->msg_callback((handle)->msg_callback_arg, SEPOL_MSG_ERR, fmt, ##__VA_ARGS__); \
} while (0)
#endif

// Entry: mandatory direct pass-through to the vulnerable function
int entry(sepol_handle_t *handle,
          sepol_policydb_t *p,
