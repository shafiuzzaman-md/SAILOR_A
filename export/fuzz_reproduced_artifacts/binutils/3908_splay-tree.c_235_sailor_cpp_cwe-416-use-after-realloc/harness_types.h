/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/splay_harness.c */
#include <stddef.h>
#include <stdlib.h>

/* Minimal type definitions to satisfy function signatures and field accesses */
typedef struct splay_tree_node_s *splay_tree_node;
struct splay_tree_node_s {
    splay_tree_node left;
    splay_tree_node right;
};

typedef struct splay_tree_s {
    splay_tree_node root;
} *splay_tree;

typedef int (*splay_tree_foreach_fn)(splay_tree_node, void *);

/* Minimal X* allocation macros used by the helper */
#ifndef INITIAL_STACK_SIZE
#define INITIAL_STACK_SIZE 100
#endif
#ifndef XNEWVEC
#define XNEWVEC(T, N) ((T*)malloc(sizeof(T) * (N)))
#endif
#ifndef XRESIZEVEC
#define XRESIZEVEC(T, P, N) ((T*)realloc((P), sizeof(T) * (N)))
#endif
#ifndef XDELETEVEC
#define XDELETEVEC(P) free((P))
#endif

