#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

// Allocator callbacks matching FT_MemoryRec_
static void* my_alloc(FT_Memory mem, FT_Long size) {
    (void)mem;
    if (size < 0) size = 0;
    return malloc((size_t)size);
}

// Realloc that returns the SAME pointer without resizing to simulate buggy behavior
static void* my_realloc(FT_Memory mem, FT_Long cur_size, FT_Long new_size, void* block) {
    (void)mem; (void)cur_size; (void)new_size;
    // Do NOT change allocation size; just pretend success
    return block ? block : malloc((size_t)(new_size > 0 ? new_size : 0));
}

static void my_free(FT_Memory mem, void* block) {
    (void)mem;
    free(block);
}

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 32) return 0;
    // Set up memory manager with our custom realloc
    struct FT_MemoryRec_ memory = {0};
    memory.alloc = my_alloc;
    memory.realloc = my_realloc;
    memory.free = my_free;

    // Concrete sizes to avoid symbolic malloc sizes
    FT_Long item_size = 4;      // bytes per item
    FT_Long cur_count = 8;      // current items -> 32 bytes allocated
    FT_Long new_count = 40;     // new items -> 160 bytes desired (new > cur)

    // Allocate initial block with cur_count * item_size bytes
    void* block = malloc((size_t)(cur_count * item_size));
    if (!block) return 0;
    // Make contents symbolic (optional; safe)
    { memcpy(block, fuzz_data + 0, 32); };

    // Call entry (pass-through to vulnerable function)
    (void)ft_mem_realloc(&memory, item_size, cur_count, new_count, block);

    return 0;
}
