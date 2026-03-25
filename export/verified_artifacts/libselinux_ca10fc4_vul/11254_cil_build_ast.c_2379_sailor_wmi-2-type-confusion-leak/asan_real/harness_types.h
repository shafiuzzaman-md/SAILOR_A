/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// Minimal enums/defines
#define SEPOL_OK 0
#define SEPOL_ERR -1

enum cil_log_level { CIL_ERR = 1 };

enum cil_flavor { CIL_TYPE_RULE = 1 };

// Types (cil_tree_node from source, cil_type_rule minimal for our path)
struct cil_tree_node {
    struct cil_tree_node *parent;
    struct cil_tree_node *cl_head;  // Head of child_list
    struct cil_tree_node *cl_tail;  // Tail of child_list
    struct cil_tree_node *next;     // Next element in the list
    enum cil_flavor flavor;
    uint32_t line;
    uint32_t hll_offset;
    void *data;
};

struct cil_type_rule {
    uint32_t rule_kind;
    char *src_str;
    char *tgt_str;
    char *obj_str;
    char *result_str;
};

