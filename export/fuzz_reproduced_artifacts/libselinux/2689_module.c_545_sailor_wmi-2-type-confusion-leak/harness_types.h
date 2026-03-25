/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>

// Minimal project-specific types used on the path
struct sepol_handle {
    void *msg_callback;      // function pointer in real code; kept as void* to avoid calling through it
    void *msg_callback_arg;
};

struct policy_file {
    struct sepol_handle *handle;
};

struct sepol_policy_file {
    struct policy_file pf;
};

typedef struct sepol_module_package_t {
    size_t seusers_len;
    char *seusers;
} sepol_module_package_t;

