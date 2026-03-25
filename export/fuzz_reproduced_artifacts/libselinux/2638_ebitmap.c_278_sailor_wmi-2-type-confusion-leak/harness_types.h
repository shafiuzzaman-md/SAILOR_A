/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

#ifndef ENOMEM
#define ENOMEM 12
#endif

// Minimal local type defs sufficient for this harness
typedef struct ebitmap_node {
    uint32_t startbit;
    unsigned long map;
    struct ebitmap_node *next;
} ebitmap_node_t;

typedef struct ebitmap {
    ebitmap_node_t *node;
    unsigned int highbit;
} ebitmap_t;

