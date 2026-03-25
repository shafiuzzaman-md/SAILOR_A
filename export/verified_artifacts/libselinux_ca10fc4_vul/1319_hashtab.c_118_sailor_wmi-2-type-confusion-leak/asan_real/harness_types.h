/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

// Minimal local definitions matching libsepol hashtab API

typedef void *hashtab_key_t;
typedef const void *const_hashtab_key_t;
typedef void *hashtab_datum_t;

struct hashtab_node {
    hashtab_key_t key;
    hashtab_datum_t datum;
    struct hashtab_node *next;
};

typedef struct hashtab_node *hashtab_ptr_t;

typedef struct hashtab *hashtab_t;

typedef unsigned int (*hashtab_hash_t)(hashtab_t h, const_hashtab_key_t key);
typedef int (*hashtab_keycmp_t)(hashtab_t h, const_hashtab_key_t a, const_hashtab_key_t b);

struct hashtab {
    hashtab_hash_t hash_value;
    hashtab_keycmp_t keycmp;
    unsigned int size;
    unsigned int nel;
    hashtab_ptr_t *htable;
};

// Vulnerable function (neutralized) — keep exact vulnerable statement text
