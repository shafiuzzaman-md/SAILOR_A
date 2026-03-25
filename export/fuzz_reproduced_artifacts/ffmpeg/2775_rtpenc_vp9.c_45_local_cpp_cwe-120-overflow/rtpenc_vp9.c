#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#ifndef RTP_VP9_DESC_REQUIRED_SIZE
#define RTP_VP9_DESC_REQUIRED_SIZE 1
#endif
#ifndef FFMIN
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#endif

typedef struct AVFormatContext {
    void *priv_data;
} AVFormatContext;

typedef struct RTPMuxContext {
    uint32_t cur_timestamp;
    uint32_t timestamp;
    uint8_t *buf;
    uint8_t *buf_ptr;
    int max_payload_size;
} RTPMuxContext;

// External stubbed elsewhere
void ff_rtp_send_data(AVFormatContext *ctx, const uint8_t *buf, int len, int last);

// Vulnerable function (neutralized minimal body, but preserving the vulnerable statement verbatim)
void ff_rtp_send_vp9(AVFormatContext *ctx, const uint8_t *buf, int size)
{
    RTPMuxContext *rtp_ctx = (RTPMuxContext *)ctx->priv_data;
    int len;

    rtp_ctx->timestamp  = rtp_ctx->cur_timestamp;
    rtp_ctx->buf_ptr    = rtp_ctx->buf;

    /* mark the first fragment */
    *rtp_ctx->buf_ptr++ = 0x08;

    // Collapse the loop to a single iteration for slicing simplicity
    if (size > 0) {
        len = FFMIN(size, rtp_ctx->max_payload_size - RTP_VP9_DESC_REQUIRED_SIZE);

        if (len == size) {
            /* mark the last fragment */
            rtp_ctx->buf[0] |= 0x04;
        }

        // Vulnerable statement copied verbatim from source
        memcpy(rtp_ctx->buf_ptr, buf, len);
        // Universal sink assertion (after the vulnerable statement)
        klee_assert(0 && "SAILOR_SINK_REACHED");

        ff_rtp_send_data(ctx, rtp_ctx->buf, len + RTP_VP9_DESC_REQUIRED_SIZE, size == len);

        // clear the end bit (kept for fidelity but unreachable after assert)
        rtp_ctx->buf[0] &= ~0x08;
    }
}

// Entry function: simple pass-through, no guards
int sail_entry(AVFormatContext *ctx, const uint8_t *buf, int size) {
    ff_rtp_send_vp9(ctx, buf, size);
    return 0;
}
