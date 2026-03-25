/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef SEPOL_OK
#define SEPOL_OK 0
#endif
#ifndef SEPOL_ERR
#define SEPOL_ERR 1
#endif

// Minimal project types needed
enum cil_flavor { CIL_FLAVOR_DUMMY = 0 };

struct cil_tree_node {
    struct cil_tree_node *parent;
    struct cil_tree_node *cl_head;    // Head of child_list
    struct cil_tree_node *cl_tail;    // Tail of child_list
    struct cil_tree_node *next;       // Each element in the list points to the next element
    enum cil_flavor flavor;
    uint32_t line;
    uint32_t hll_offset;
    void *data;
};

struct cil_db { int _pad; };

struct cil_perm { unsigned int value; };

// Vulnerable function (neutralized) — keep signature and vulnerable statement verbatim
