/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal needed types

typedef unsigned int uint32_t;

enum cil_flavor { CIL_FLAVOR_DUMMY = 0 };
#ifndef CIL_BLOCKABSTRACT
#define CIL_BLOCKABSTRACT 777
#endif

struct cil_tree_node {
    struct cil_tree_node *parent;
    struct cil_tree_node *cl_head;
    struct cil_tree_node *cl_tail;
    struct cil_tree_node *next;
    enum cil_flavor flavor;
    uint32_t line;
    uint32_t hll_offset;
    void *data;
};

struct cil_blockabstract {
    char *block_str;
};

struct cil_db {
    int qualified_names;
};

