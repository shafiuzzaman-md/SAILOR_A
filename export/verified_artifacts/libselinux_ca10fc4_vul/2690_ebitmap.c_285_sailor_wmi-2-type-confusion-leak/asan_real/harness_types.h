/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for ebitmap_cpy vulnerability at ebitmap.c:285 */
#include <stdlib.h>
#include <stdint.h>

/* Minimal local definitions to match usage in ebitmap_cpy */
typedef struct ebitmap_node {
    unsigned int startbit;
    unsigned long map[1];
    struct ebitmap_node *next;
} ebitmap_node_t;

typedef struct ebitmap {
    ebitmap_node_t *node;
    unsigned int highbit;
} ebitmap_t;

