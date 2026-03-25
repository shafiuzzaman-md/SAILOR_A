// Combined reproducer for 6933_aacenc_ltp.c_58_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
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
