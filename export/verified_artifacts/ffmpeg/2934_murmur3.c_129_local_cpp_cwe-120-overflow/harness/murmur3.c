#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <klee/klee.h>

#ifndef STATE_SIZE
#define STATE_SIZE 16
#endif

typedef struct AVMurMur3 {
    uint64_t h1;
    uint64_t h2;
    uint8_t  state[STATE_SIZE];
    int      state_pos;
    size_t   len;
} AVMurMur3;

// Vulnerable function (neutralized, keep only the sink path)
void av_murmur3_update(AVMurMur3 *c, const uint8_t *src, size_t len) {
    // Neutralized body: skip the main loop and keep only the tail handling
    len &= 15;
    if (len > 0) {
        // Vulnerable statement — keep EXACT text as in source_context
        memcpy(c->state, src, len);
        c->state_pos = (int)len;
        // Universal sink assertion (after the vulnerable statement)
        klee_assert(0 && "SAILOR_SINK_REACHED");
    }
}

// Mandatory neutralized entry — direct pass-through, no guards
int entry_func(AVMurMur3 *c, const uint8_t *src, size_t len) {
    av_murmur3_update(c, src, len);
    return 0;
}
