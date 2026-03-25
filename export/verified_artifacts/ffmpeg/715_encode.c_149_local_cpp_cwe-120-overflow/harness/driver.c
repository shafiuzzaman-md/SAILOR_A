
#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>
#include <string.h>

#ifndef PKT_BUFSZ
#define PKT_BUFSZ 32
#endif
#ifndef CTX_BUFSZ
#define CTX_BUFSZ 96
#endif

int avcodec_receive_packet(AVCodecContext *avctx, AVPacket *avpkt);

int main() {
    // 1) Concrete struct allocations
    AVCodecContext *avctx = (AVCodecContext *)calloc(1, sizeof(AVCodecContext));
    AVPacket *avpkt = (AVPacket *)calloc(1, sizeof(AVPacket));

    // 2) Concrete buffers + symbolic contents
    unsigned char *pkt_buf = (unsigned char *)malloc(PKT_BUFSZ);
    unsigned char *ctx_buf = (unsigned char *)malloc(CTX_BUFSZ);
    klee_make_symbolic(pkt_buf, PKT_BUFSZ, "pkt_buf");
    klee_make_symbolic(ctx_buf, CTX_BUFSZ, "ctx_buf");

    // 3) Wire fields
    avpkt->data = pkt_buf;
    avpkt->size = PKT_BUFSZ;
    avctx->internal_buffer = ctx_buf;

    // 4) Control memcpy length via a local symbolic variable, then assign
    int sym_len;
    klee_make_symbolic(&sym_len, sizeof(sym_len), "internal_size");
    klee_assume(sym_len >= 0);
    klee_assume(sym_len <= 2 * CTX_BUFSZ);
    avctx->internal_size = sym_len;

    // 5) Call entry (pass-through) into vulnerable slice
    avcodec_receive_packet(avctx, avpkt);
    return 0;
}
