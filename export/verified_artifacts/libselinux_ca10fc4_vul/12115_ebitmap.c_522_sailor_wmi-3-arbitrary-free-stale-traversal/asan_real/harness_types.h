/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>

// Minimal local type definitions sufficient for ebitmap_destroy
// Keep only fields referenced by the vulnerable function.
typedef struct ebitmap_node_t {
    struct ebitmap_node_t *next;
} ebitmap_node_t;

typedef struct ebitmap_t {
    ebitmap_node_t *node;
    unsigned int highbit;
} ebitmap_t;

// Vulnerable function (verbatim body with universal sink after the vulnerable statement)
