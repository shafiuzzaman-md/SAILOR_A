/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c — minimal neutralized harness for cil_resolve_sidorder */
#include <stdint.h>
#include <stdlib.h>

/* Minimal local type definitions needed by the harness */
struct cil_list_item { void *data; struct cil_list_item *next; };
struct cil_list { struct cil_list_item *head; };
struct cil_ordered { struct cil_list *strs; struct cil_list *datums; };

enum cil_flavor { CIL_FLAVOR_DUMMY = 0 };
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

struct cil_db { int dummy; };

/* Define the macro exactly as a plausible expansion, and keep the original call site text verbatim below. */
#ifndef cil_list_for_each
#define cil_list_for_each(curr, list) \
    for ((curr) = ((list) ? (list)->head : NULL); (curr) != NULL; (curr) = (curr)->next)
#endif

/* Vulnerable function — neutralized to the essential pattern. Keep the vulnerable statement verbatim. */
