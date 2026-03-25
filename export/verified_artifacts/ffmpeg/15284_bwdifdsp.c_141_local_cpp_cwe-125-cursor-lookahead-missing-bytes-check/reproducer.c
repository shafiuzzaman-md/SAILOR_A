// Combined reproducer for 15284_bwdifdsp.c_141_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: FFABS (auto-detected external) */
int FFABS() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: probe (auto-detected external) */
int probe() { return 0; }

/* PROACTIVE: statement (auto-detected external) */
int statement() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// Prototype from harness
int entry_func(void *dst1, const void *prev1, const void *cur1, const void *next1,
               int w, int prefs, int mrefs, int prefs2, int mrefs2,
               int prefs3, int mrefs3, int prefs4, int mrefs4,
               int parity, int clip_max);

int main() {
    // Concrete sizes per instructions (no symbolic malloc sizes)
    const int SZ = 16;

    // Allocate buffers
    uint8_t *dst  = (uint8_t *)malloc(SZ);
    uint8_t *prev = (uint8_t *)malloc(SZ);
    uint8_t *cur  = (uint8_t *)malloc(SZ);
    uint8_t *next = (uint8_t *)malloc(SZ);

    // Make contents symbolic (not strictly needed but harmless)
    klee_make_symbolic(dst,  SZ, "dst_buf");
    klee_make_symbolic(prev, SZ, "prev_buf");
    klee_make_symbolic(cur,  SZ, "cur_buf");
    klee_make_symbolic(next, SZ, "next_buf");

    // Set indices small and within bounds for cur/next
    int w = 8;           // unused in our slice
    int prefs = 1, mrefs = 1;
    int prefs2 = 2, mrefs2 = 2;
    int prefs3 = 3, mrefs3 = 3;
    int prefs4 = 4, mrefs4 = 4;
    int clip_max = 255;

    // parity selects prev2 = prev when non-zero
    int parity = 1;

    // OOB setup: make prev2 point one past the allocated buffer
    const uint8_t *prev_oob = prev + SZ; // prev2[0] will be OOB

    // Call entry (pass-through to vulnerable function)
    entry_func(dst, prev_oob, cur, next,
               w, prefs, mrefs, prefs2, mrefs2,
               prefs3, mrefs3, prefs4, mrefs4,
               parity, clip_max);

    return 0;
}
