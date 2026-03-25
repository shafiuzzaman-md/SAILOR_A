#include "harness_types.h"
// klee removed

unsigned int stub_hash_value(hashtab_t h, const_hashtab_key_t key) {
    unsigned int v;
    memset(&v, sizeof(v), "hash_value_ret") /* stub */;;
    // Keep within small range to index initial table without reading h
    
    return v;
}

int stub_keycmp(hashtab_t h, const_hashtab_key_t key1, const_hashtab_key_t key2) {
    int r;
    memset(&r, sizeof(r), "keycmp_ret") /* stub */;;
    // No constraints; KLEE explores ordering and equality paths
    return r;
}
