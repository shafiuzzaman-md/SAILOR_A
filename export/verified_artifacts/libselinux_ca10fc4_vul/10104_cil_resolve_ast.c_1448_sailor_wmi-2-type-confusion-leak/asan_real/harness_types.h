/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stddef.h>

// Minimal type definitions needed by the harness
struct cil_list_item {
    struct cil_list_item *next;
    int flavor;  // enum cil_flavor simplified
    void *data;
};

struct cil_list {
    struct cil_list_item *head;
    struct cil_list_item *tail;
    int flavor; // enum cil_flavor simplified
};

#define cil_list_for_each(item, list) \
    for (item = (list)->head; item != NULL; item = item->next)

struct cil_ordered {
    struct cil_list *strs;
    struct cil_list *datums; // not used by harness but present in original
};

struct cil_tree_node {
    void *data;  // points to struct cil_ordered
};

struct cil_db { int dummy; };

// Vulnerable function signature preserved
