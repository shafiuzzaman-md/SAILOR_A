/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>

// Minimal type definitions required by the harness and driver
typedef struct sepol_module_package {
    size_t file_contexts_len;
    char *file_contexts;
    size_t seusers_len;
    char *seusers;
} sepol_module_package_t;

struct sepol_handle {
    void (*msg_callback)(void *arg, const char *fmt, ...);
    void *msg_callback_arg;
};

struct policy_file {
    struct sepol_handle *handle;
};

struct sepol_policy_file {
    struct policy_file pf;
};

// Define ERR as in-source callback through the handle fields.
#ifndef ERR
#define ERR(h, fmt, ...) do { \
    if ((h) && (h)->msg_callback) \
        (h)->msg_callback((h)->msg_callback_arg, fmt, ##__VA_ARGS__); \
} while (0)
#endif

// Neutralized vulnerable function: keep signature and the exact vulnerable statement
int sepol_module_package_read(sepol_module_package_t *mod,
