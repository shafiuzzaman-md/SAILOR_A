/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stdint.h>
#include <stdlib.h>

struct cil_db; /* opaque; not dereferenced in harness */

enum cil_flavor { CIL_FLAVOR_DUMMY = 0 };

/* From project source (GatherCode) */
struct cil_tree_node {
    struct cil_tree_node *parent;
    struct cil_tree_node *cl_head;      // Head of child_list
    struct cil_tree_node *cl_tail;      // Tail of child_list
    struct cil_tree_node *next;         // Each element in the list points to the next element
    enum cil_flavor flavor;
    uint32_t line;
    uint32_t hll_offset;
    void *data;
};

/* Vulnerable function — neutralized to minimal path. Keep the exact vulnerable statement. */
