/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>

// Minimal local types matching hashtab.h usage

typedef void *hashtab_datum_t;
typedef void *hashtab_key_t;
typedef const void *const_hashtab_key_t;

struct hashtab_list;
typedef struct hashtab_list *hashtab_ptr_t;

typedef struct hashtab *hashtab_t;

struct hashtab_list {
    hashtab_key_t key;
    hashtab_datum_t datum;
    hashtab_ptr_t next;
};

struct hashtab {
    unsigned int size;
    hashtab_ptr_t *htable;
    unsigned int (*hash_value) (hashtab_t h, const_hashtab_key_t key);
    int (*keycmp) (hashtab_t h, const_hashtab_key_t key1, const_hashtab_key_t key2);
};

// === Neutralized vulnerable function (KEEP exact statements; add sink after vulnerable read) ===
