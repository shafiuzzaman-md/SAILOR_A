#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// entry_func prototype from harness
int cond_av_list_destroy(cond_av_list_t *list);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Allocate two nodes
    cond_av_list_t *a = (cond_av_list_t *)malloc(sizeof(cond_av_list_t));
    cond_av_list_t *b = (cond_av_list_t *)malloc(sizeof(cond_av_list_t));

    // Initialize
    a->next = NULL;
    // Make b's contents symbolic (not strictly necessary, but harmless)
    { memcpy(b, fuzz_data + 0, 8); };
    b->next = NULL;

    // Create a stale reference: a->next points to b, then free(b)
    a->next = b;
    free(b);  // b is freed independently; a->next is now stale

    // Call into the entry function which will traverse and dereference b
    cond_av_list_destroy(a);

    return 0;
}
