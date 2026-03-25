#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Define the global offset used by the harness entry to call index_load
uint32_t g_offset = 1;

extern fz_buffer *fz_subset_cff_for_gids(fz_context *ctx, fz_buffer *orig, int *gids, int num_gids, int symbolic, int cidfont);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 4) return 0;
    // Allocate context and buffer
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_buffer *orig = (fz_buffer *)calloc(1, sizeof(fz_buffer));

    // Concrete allocation size (must be concrete, and > DICT_MAX_ARGS)
    const size_t buflen = 4; // small to force OOB at data[2]
    unsigned char *buf = (unsigned char *)malloc(buflen);

    // Make buffer content symbolic
    { memcpy(buf, fuzz_data + 0, 4); };

    // Symbolic length within [1, buflen]
    unsigned int sym_len;
    { static const unsigned char orig_len_data[] = {0x04, 0x00, 0x00, 0x00}; memcpy(&sym_len, orig_len_data, (sizeof(sym_len) < sizeof(orig_len_data)) ? sizeof(sym_len) : sizeof(orig_len_data)); };
    /* klee_assume removed */
    /* klee_assume removed */

    // Set up buffer struct (satisfy entry preconditions)
    orig->data = buf;
    orig->len = sym_len;

    // Drive offset: keep small and non-zero to bypass early return (offset==0)
    { static const unsigned char g_offset_data[] = {0x03, 0x00, 0x00, 0x00}; memcpy(&g_offset, g_offset_data, (sizeof(g_offset) < sizeof(g_offset_data)) ? sizeof(g_offset) : sizeof(g_offset_data)); };
    /* klee_assume removed */
    /* klee_assume removed */ // small range keeps exploration feasible

    // Call entry (neutralized pass-through to index_load)
    (void)fz_subset_cff_for_gids(ctx, orig, NULL, 0, 0, 0);

    return 0;
}
