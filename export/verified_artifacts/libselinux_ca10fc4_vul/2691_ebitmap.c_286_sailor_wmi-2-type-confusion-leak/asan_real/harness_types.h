/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Minimal local type definitions sufficient for the slice

typedef struct ebitmap_node {
    unsigned long startbit;
    unsigned long long map; // width is not critical for harness fidelity
    struct ebitmap_node *next;
} ebitmap_node_t;

typedef struct ebitmap {
    ebitmap_node_t *node;
    unsigned long highbit;
} ebitmap_t;

// Minimal helpers referenced by ebitmap_cpy
