/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// Minimal local type model sufficient for the ERR() macro dereference path
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif
#ifndef STATUS_ERR
#define STATUS_ERR -1
#endif

// Simplified sepol handle structure capturing fields used by ERR path
typedef void (*sepol_msg_callback_t)(void *arg, int level, const char *fmt, va_list ap);
typedef struct sepol_handle {
    sepol_msg_callback_t msg_callback;   // read by ERR path
    void *msg_callback_arg;              // read by ERR path
    int msg_level;                       // may be read in real ERR; keep for fidelity
} sepol_handle_t;

// Dummy ibendport/context types — body not accessed in neutralized slice
typedef struct sepol_ibendport {
    char *ibdev_name;
    void *con;
    int port;
} sepol_ibendport_t;

typedef struct sepol_context { int dummy; } sepol_context_t;

// Define a local ERR() that dereferences fields through the handle, matching real semantics that
