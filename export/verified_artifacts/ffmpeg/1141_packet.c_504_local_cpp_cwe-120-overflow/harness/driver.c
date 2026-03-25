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
