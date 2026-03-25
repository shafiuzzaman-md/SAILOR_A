/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

/* Minimal types to satisfy the entry function signature */
typedef struct htab {
    void **entries;
    size_t size;
    unsigned int size_prime_index;
    void *hash_f;
    void *eq_f;
    void *del_f;
    void *alloc_arg;
    void *alloc_with_arg_f;
    void *free_with_arg_f;
} *htab_t;

typedef unsigned int (*htab_hash)(const void *);
typedef int (*htab_eq)(const void *, const void *);
typedef void (*htab_del)(void *);
typedef void *(*htab_alloc_with_arg)(void *alloc_arg, size_t, size_t);
typedef void (*htab_free_with_arg)(void *alloc_arg, void *ptr);

/* Minimal prime table to drive the binary search. Keeping only the 'prime' field. */
struct prime_ent { unsigned long prime; };
static const struct prime_ent prime_tab[] = {
    {5UL}, {7UL}
};

/* Vulnerable function: keep the control skeleton and the exact vulnerable statement. */
