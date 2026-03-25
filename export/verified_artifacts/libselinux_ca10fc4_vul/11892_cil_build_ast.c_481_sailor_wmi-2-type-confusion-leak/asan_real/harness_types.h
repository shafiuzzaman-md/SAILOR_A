/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal type stubs needed for the vulnerable path
struct cil_db;  // opaque, unused in neutralized slice

enum cil_flavor { CIL_BLOCKABSTRACT = 1001 };

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
    void *block_str;  // assign from parse_current->next->data (void*)
};

