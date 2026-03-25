#include <stddef.h>
// Combined reproducer for 1146_packet.c_453_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: AVERROR (auto-detected external) */
int AVERROR() { return 0; }

/* PROACTIVE: av_assert1 (auto-detected external) */
int av_assert1() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Declarations from harness
int entry_func(AVPacket *dst, const AVPacket *src);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Concrete buffer sizes (no symbolic malloc sizes)
    enum { SRC_DATA_CAP = 16, COPY_SIZE = 64 };

    // Allocate packets
    AVPacket *src = (AVPacket*)calloc(1, sizeof(AVPacket));
    AVPacket *dst = (AVPacket*)calloc(1, sizeof(AVPacket));

    // Ensure we take the !src->buf path
    src->buf = NULL;

    // Allocate a smaller source buffer than the memcpy length to trigger OOB read
    uint8_t *src_data = (uint8_t*)malloc(SRC_DATA_CAP);
    memcpy(src_data, fuzz_data + (0), SRC_DATA_CAP);
    src->data = src_data;

    // Set size larger than allocated source to cause overflow during memcpy
    src->size = COPY_SIZE; // 64 > 16

    // Call entry to reach vulnerable memcpy
    entry_func(dst, src);

    return 0;
}
