#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <klee/klee.h>

// Minimal type definitions needed for the harness
typedef struct AACEncContext {
    const uint8_t *chan_map;  // channel configuration map
} AACEncContext;

// Vulnerable function (neutralized): keep only the vulnerable statement verbatim
void ff_aac_ltp_insert_new_frame(AACEncContext *s)
{
    int i;
    // Vulnerable lookahead read in loop condition — verbatim from source
    for (i = 0; i < s->chan_map[0]; i++) {
        // Universal sink assertion: if the OOB doesn't crash, mark reachability
        klee_assert(0 && "SAILOR_SINK_REACHED");
    }
}

// Entry function: strict pass-through to the vulnerable function
int aac_entry(AACEncContext *s) {
    ff_aac_ltp_insert_new_frame(s);
    return 0;
}
