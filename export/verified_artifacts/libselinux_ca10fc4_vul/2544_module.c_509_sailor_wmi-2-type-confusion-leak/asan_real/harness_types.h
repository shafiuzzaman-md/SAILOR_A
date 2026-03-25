/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef le32_to_cpu
#define le32_to_cpu(x) (x)
#endif

/* Minimal type definitions required for the slice */
typedef struct sepol_handle sepol_handle;

typedef void (*sepol_msg_callback_t)(void *arg, const char *msg);

struct sepol_handle {
    sepol_msg_callback_t msg_callback;
    void *msg_callback_arg;
};

struct policy_file {
    sepol_handle *handle;
};

struct sepol_policy_file {
    struct policy_file pf;
};

typedef struct sepol_module_package_t {
    /* Only keep fields referenced by this slice if needed */
    size_t file_contexts_len;
    char *file_contexts;
} sepol_module_package_t;

/* Constants needed by the switch */
#ifndef SEPOL_PACKAGE_SECTION_FC
#define SEPOL_PACKAGE_SECTION_FC 1
#endif

/* Flags for seen sections */
#ifndef SEEN_FC
#define SEEN_FC  2
#endif

