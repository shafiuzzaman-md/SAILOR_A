/* AUTO-GENERATED from harness preamble */
#pragma once

/* minimal sliced harness for node_record.c: sepol_node_get_mask_bytes */
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

/* Local minimal type defs to avoid project header chains */
typedef struct sepol_handle sepol_handle_t;  /* opaque */
typedef struct sepol_context sepol_context_t; /* opaque */

typedef struct sepol_node {
    /* Only fields needed by the target function (keep layout simple) */
    char *addr; size_t addr_sz;  /* not used here but present in real struct */
    char *mask; size_t mask_sz;  /* used by target */
    int proto;                   /* not used here */
    sepol_context_t *con;        /* not used here */
} sepol_node_t;

#ifndef STATUS_ERR
#define STATUS_ERR (-1)
#endif
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS (0)
#endif
