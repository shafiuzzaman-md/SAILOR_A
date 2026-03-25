#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>

// Prototype from harness
int aac_entry(struct AACEncContext *s);

int main() {
    // Allocate context
    struct AACEncContext *s = (struct AACEncContext *)calloc(1, sizeof(*s));

    // Backing buffer for chan_map
    enum { BUF_SZ = 128 };
    uint8_t *buf = (uint8_t *)malloc(BUF_SZ);
    klee_make_symbolic(buf, BUF_SZ, "chan_map_buf");

    // Point chan_map one-past-end to force OOB read on chan_map[0]
    s->chan_map = (const uint8_t *)(buf + BUF_SZ);

    // Call entry (pass-through to vulnerable function)
    return aac_entry(s);
}
