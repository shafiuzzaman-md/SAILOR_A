/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

// Minimal type shims to satisfy signatures
typedef struct { int _dummy; } policydb_t;

struct cil_list_item {
    struct cil_list_item *next;
    int flavor;
    void *data;
};

struct cil_list {
    struct cil_list_item *head;
};

struct cil_db {
    // Minimal definition: only the field used at the vulnerable site
    struct cil_list *sensitivityorder;
};

#define cil_list_for_each(item, list) \
    for (item = (list)->head; item != NULL; item = item->next)

// Neutralized vulnerable function: keep signature and the exact vulnerable statement
