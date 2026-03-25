/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifndef EXPR_BUF_SIZE
#define EXPR_BUF_SIZE 1024
#endif
#ifndef REASON_BUF_SIZE
#define REASON_BUF_SIZE 2048
#endif

#ifndef PF_USE_MEMORY
#define PF_USE_MEMORY  0
#endif
#ifndef PF_USE_STDIO
#define PF_USE_STDIO   1
#endif
#ifndef PF_LEN
#define PF_LEN         2
#endif

struct sepol_handle; // opaque

struct policy_file {
    unsigned type;
    char *data;
    size_t len;
    size_t size;
    FILE *fp;
    struct sepol_handle *handle;
};

typedef struct policy_file policy_file_t;

