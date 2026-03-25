/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Minimal types to exercise the vulnerable statement

typedef void (*sepol_msg_callback_t)(void *arg, const char *fmt, ...);

typedef struct sepol_handle {
    sepol_msg_callback_t msg_callback;
    void *msg_callback_arg;
} sepol_handle;

// Minimal ERR macro that touches handle->msg_callback and ->msg_callback_arg
#ifndef ERR
#define ERR(handle, fmt, ...)                                                     \
    do {                                                                          \
        if ((handle) && (handle)->msg_callback) {                                 \
            (handle)->msg_callback((handle)->msg_callback_arg, fmt, ##__VA_ARGS__);\
        }                                                                         \
    } while (0)
#endif

