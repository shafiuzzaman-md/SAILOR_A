#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Prototypes from harness
int hashtab_search(hashtab_t h, const_hashtab_key_t key);
void hashtab_destroy(hashtab_t h);

// Stub prototypes (implemented in stubs.c)
unsigned int stub_hash_value(hashtab_t h, const_hashtab_key_t key);
int stub_keycmp(hashtab_t h, const_hashtab_key_t key1, const_hashtab_key_t key2);

int main() {
    // 1) Allocate hashtab object concretely
    struct hashtab *hh = (struct hashtab*)calloc(1, sizeof(struct hashtab));
    hashtab_t h = hh;

    // 2) Initialize fields
    h->size = 4; // small, concrete size
    h->htable = (hashtab_ptr_t*)calloc(h->size, sizeof(hashtab_ptr_t));

    // 3) Assign function pointers to symbolic-return stubs
    h->hash_value = stub_hash_value;
    h->keycmp = stub_keycmp;

    // 4) Prepare a simple bucket with symbolic contents (not strictly required for UAF)
    struct hashtab_list *node = (struct hashtab_list*)calloc(1, sizeof(struct hashtab_list));
    char *kbuf = (char*)malloc(16);
    { static const unsigned char node_key_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(kbuf, node_key_data, (16 < sizeof(node_key_data)) ? 16 : sizeof(node_key_data)); };
    node->key = (void*)kbuf;
    char *dbuf = (char*)malloc(8);
    { static const unsigned char node_datum_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(dbuf, node_datum_data, (8 < sizeof(node_datum_data)) ? 8 : sizeof(node_datum_data)); };
    node->datum = (void*)dbuf;
    node->next = NULL;
    h->htable[0] = node;

    // 5) Prepare search key buffer
    char *search_key = (char*)malloc(16);
    { static const unsigned char search_key_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(search_key, search_key_data, (16 < sizeof(search_key_data)) ? 16 : sizeof(search_key_data)); };

    // 6) Free the hashtab (UAF setup), then use it via hashtab_search() to trigger dereference of freed memory
    hashtab_destroy(h);
    hashtab_search(h, (const void*)search_key);

    return 0;
}
