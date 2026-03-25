#include "harness_types.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// Entry from harness
int sail_entry(AVFormatContext *ctx, const uint8_t *buf, int size);

int main() {
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
    klee_make_symbolic(in, 64, "vp9_input");

    // Choose a size large enough to exceed rtp->buf capacity after header byte
    int size = 50;  // len = min(50, 39) = 39 → writes 39 bytes at buf+1 → overflows 16-byte buf

    // Call entry directly
    sail_entry(ctx, in, size);
    return 0;
}
