/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Local type model for libsepol handle
typedef struct sepol_handle {
    int (*msg_callback)(void *varg, const char *fmt, ...);
    void *msg_callback_arg;
} sepol_handle_t;

