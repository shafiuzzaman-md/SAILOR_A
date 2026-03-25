#include <stddef.h>
// Combined reproducer for 2775_rtpenc_vp9.c_45_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: FFMIN (auto-detected external) */
int FFMIN() { return 0; }

/* PROACTIVE: assertion (auto-detected external) */
int assertion() { return 0; }

/* PROACTIVE: bit (auto-detected external) */
int bit() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// Entry from harness
int sail_entry(AVFormatContext *ctx, const uint8_t *buf, int size);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate context structs concretely
    AVFormatContext *ctx = (AVFormatContext *)calloc(1, sizeof(AVFormatContext));
    RTPMuxContext *rtp = (RTPMuxContext *)calloc(1, sizeof(RTPMuxContext));

    // Wire priv_data
    ctx->priv_data = rtp;

    // Allocate a SMALL RTP buffer to trigger overflow in memcpy
    // Must be > RTP_VP9_DESC_REQUIRED_SIZE (1) per note
    rtp->buf = (uint8_t *)malloc(16);

    // Set payload size LARGE so len becomes large
    rtp->max_payload_size = 40;  // so len can be up to 39

    // Timestamp can be any value
    rtp->cur_timestamp = 0x12345678;

    // Prepare input buffer (source) with sufficient size and symbolic contents
    uint8_t *in = (uint8_t *)malloc(64);
    memcpy(in, fuzz_data + (0), 64);

    // Choose a size large enough to exceed rtp->buf capacity after header byte
    int size = 50;  // len = min(50, 39) = 39 → writes 39 bytes at buf+1 → overflows 16-byte buf

    // Call entry directly
    sail_entry(ctx, in, size);
    return 0;
}
