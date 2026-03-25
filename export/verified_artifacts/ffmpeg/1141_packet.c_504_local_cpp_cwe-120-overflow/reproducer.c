// Combined reproducer for 1141_packet.c_504_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: ENOMEM (auto-detected external) */
int ENOMEM() { return 0; }

/* PROACTIVE: FUNCTION (auto-detected external) */
int FUNCTION() { return 0; }

/* PROACTIVE: av_assert1 (auto-detected external) */
int av_assert1() { return 0; }

/* PROACTIVE: probe (auto-detected external) */
int probe() { return 0; }

/* PROACTIVE: through (auto-detected external) */
int through() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// entry_func is defined in harness/packet.c
int entry_func(struct AVPacket *pkt);

int main() {
    // Allocate packet
    struct AVPacket *pkt = (struct AVPacket *)calloc(1, sizeof(*pkt));

    // Small concrete source buffer
    enum { SRC_SZ = 8 };
    uint8_t *src = (uint8_t *)malloc(SRC_SZ);
    if (!pkt || !src) return 0;

    // Make source content symbolic
    klee_make_symbolic(src, SRC_SZ, "src_bytes");

    // Make size symbolic but constrained to be larger than SRC_SZ
    int sz;
    klee_make_symbolic(&sz, sizeof(sz), "pkt_size");
    klee_assume(sz > SRC_SZ);
    klee_assume(sz <= 64);
    klee_assume(sz > 0);

    // Initialize pkt fields to hit the vulnerable path
    pkt->buf  = NULL;        // force allocation path
    pkt->data = src;         // non-NULL source to satisfy av_assert1
    pkt->size = sz;          // unchecked length used by memcpy

    // Call entry (pass-through to av_packet_make_refcounted)
    entry_func(pkt);

    return 0;
}
