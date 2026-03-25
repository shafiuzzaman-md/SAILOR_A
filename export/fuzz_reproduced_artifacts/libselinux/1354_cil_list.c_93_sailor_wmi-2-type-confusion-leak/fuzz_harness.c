#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// Entry from harness
int cil_list_item_destroy(struct cil_list_item **item, unsigned destroy_data);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 24) return 0;
    // Phase 1: allocate a cil_list_item concretely
    struct cil_list_item *p = (struct cil_list_item *)malloc(sizeof(struct cil_list_item));
    if (!p) return 0;

    // Make contents symbolic to maximize exploration (not the pointer itself)
    { memcpy(p, fuzz_data + 0, 24); };

    // Keep an alias that will become stale after free
    struct cil_list_item *alias = p;

    // Phase 2: free original object to create a stale pointer (UAF setup)
    free(p);

    // Ensure we take the destroy_data branch
    unsigned destroy_data = 1;
    { static const unsigned char destroy_flag_data[] = {0xff, 0x00, 0x00, 0x00}; memcpy(&destroy_data, destroy_flag_data, (sizeof(destroy_data) < sizeof(destroy_flag_data)) ? sizeof(destroy_data) : sizeof(destroy_flag_data)); };
    /* klee_assume removed */

    // Phase 3: use-after-free via vulnerable function
    cil_list_item_destroy(&alias, destroy_data);
    return 0;
}
