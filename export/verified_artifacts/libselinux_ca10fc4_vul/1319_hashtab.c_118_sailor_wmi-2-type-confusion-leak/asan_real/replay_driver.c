#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Prototype from harness
int hashtab_search(hashtab_t h, const_hashtab_key_t key);

// External stubs we provide in stubs.c
unsigned int sym_hash(hashtab_t h, const_hashtab_key_t key);
int sym_keycmp(hashtab_t h, const_hashtab_key_t a, const_hashtab_key_t b);

int main() {
    // Allocate hashtab object
    hashtab_t h = (hashtab_t)calloc(1, sizeof(*h));

    // Configure table size (concrete) and allocate buckets
    unsigned int sz = 8;  // concrete size per guidance
    h->size = sz;
    h->htable = (hashtab_ptr_t *)calloc(h->size, sizeof(hashtab_ptr_t));

    // Install symbolic hash and a keycmp (not used before the sink but set anyway)
    h->hash_value = sym_hash;
    h->keycmp = sym_keycmp;

    // Prepare a symbolic key buffer
    unsigned char keybuf[16];
    { static const unsigned char keybuf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(keybuf, keybuf_data, (sizeof(keybuf) < sizeof(keybuf_data)) ? sizeof(keybuf) : sizeof(keybuf_data)); };
    const_hashtab_key_t key = (const_hashtab_key_t)keybuf;

    // WMI-2 / UAF setup: free the table object so dereferences in hashtab_search
    // operate on a freed (potentially reclaimed) object.
    free(h);

    // Direct call into entry (no guards here). Passing a stale pointer simulates UAF/type confusion.
    hashtab_search(h, key);
    return 0;
}
