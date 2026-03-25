#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

int deflateParams(void *p, int level, int strategy);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 16) return 0;
    // Allocate z_stream and deflate_state
    z_stream_local *strm = (z_stream_local *)calloc(1, sizeof(z_stream_local));
    deflate_state *s = (deflate_state *)calloc(1, sizeof(deflate_state));
    if (!strm || !s) return 0;
    strm->state = s;

    // Small head buffer to induce overflow when clearing hash
    size_t head_elems = 8; // elements of uint16_t
    uint16_t *head = (uint16_t *)malloc(head_elems * sizeof(uint16_t));
    if (!head) return 0;
    { memcpy(head, fuzz_data + 0, 16); };
    s->head = head;

    // Force path to CLEAR_HASH: level == 0, matches != 0 and != 1
    s->level = 0;
    s->matches = 2; // not 0 and not 1 to take the CLEAR_HASH branch

    // Symbolic hash_size: larger than allocated head_elems to overflow memset
    size_t hash_size;
    { static const unsigned char hash_size_data[] = {0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&hash_size, hash_size_data, (sizeof(hash_size) < sizeof(hash_size_data)) ? sizeof(hash_size) : sizeof(hash_size_data)); };
    /* klee_assume removed */
    /* klee_assume removed */ // reasonable upper bound
    s->hash_size = hash_size;

    // Level index for configuration_table must be in-bounds [0, MAX_LEVELS)
    int lvl;
    { static const unsigned char level_idx_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&lvl, level_idx_data, (sizeof(lvl) < sizeof(level_idx_data)) ? sizeof(lvl) : sizeof(level_idx_data)); };
    /* klee_assume removed */
#ifndef MAX_LEVELS
#define MAX_LEVELS 10
#endif
    /* klee_assume removed */

    int strategy = 0;

    // Direct call through the entry function
    deflateParams((void *)strm, lvl, strategy);
    return 0;
}
