/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal enum to satisfy struct field
enum cil_flavor { CIL_ROOT = 0, CIL_MIN_DECLARATIVE = 1 };

// Exact type from source (GatherCode)
struct cil_tree_node {
    struct cil_tree_node *parent;
    struct cil_tree_node *cl_head;        //Head of child_list
    struct cil_tree_node *cl_tail;        //Tail of child_list
    struct cil_tree_node *next;        //Each element in the list points to the next element
    enum cil_flavor flavor;
    uint32_t line;
    uint32_t hll_offset;
    void *data;
};

// Neutralized: keep signature and the vulnerable statement verbatim, plus sink assertion after it
