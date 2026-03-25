/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for fibheap.c vulnerability reachability */
#include <stdlib.h>
#include <stdint.h>

/* Local minimal type defs (project-independent) */
typedef struct fibnode fibnode;
typedef struct fibheap fibheap;

typedef struct fibnode *fibnode_t;
typedef struct fibheap *fibheap_t;

typedef int fibheapkey_t; /* not used here, but referenced in preamble normally */

struct fibnode {
    struct fibnode *left;
    struct fibnode *right;
    struct fibnode *parent;
    struct fibnode *child;
    int degree;
    int mark;
    void *data;
};

struct fibheap {
    fibnode_t min;
    unsigned int nodes;
};

