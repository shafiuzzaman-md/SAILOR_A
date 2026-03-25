/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stdint.h>
#include <stdlib.h>

#ifndef CIL_TRUE
#define CIL_TRUE 1
#endif
#ifndef CIL_FALSE
#define CIL_FALSE 0
#endif

/* Minimal enum to satisfy struct definitions */
enum cil_flavor { CIL_MIN_DECLARATIVE = 1 };

/* Types required by the path */
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

struct cil_classperms_set {
    char *set_str;
};

