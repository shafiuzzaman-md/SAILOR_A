#include <stdint.h>
#include <stddef.h>
// Combined reproducer for 715_encode.c_149_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===

#include "harness_types.h"
#include <stdlib.h>
#include <string.h>

#ifndef PKT_BUFSZ
#define PKT_BUFSZ 32
#endif
#ifndef CTX_BUFSZ
#define CTX_BUFSZ 96
#endif

int avcodec_receive_packet(AVCodecContext *avctx, AVPacket *avpkt);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // 1) Concrete struct allocations
    AVCodecContext *avctx = (AVCodecContext *)calloc(1, sizeof(AVCodecContext));
    AVPacket *avpkt = (AVPacket *)calloc(1, sizeof(AVPacket));

    // 2) Concrete buffers + symbolic contents
    unsigned char *pkt_buf = (unsigned char *)malloc(PKT_BUFSZ);
    unsigned char *ctx_buf = (unsigned char *)malloc(CTX_BUFSZ);
    memcpy(pkt_buf, fuzz_data + (0), PKT_BUFSZ);
    memcpy(ctx_buf, fuzz_data + (PKT_BUFSZ), CTX_BUFSZ);

    // 3) Wire fields
    avpkt->data = pkt_buf;
    avpkt->size = PKT_BUFSZ;
    avctx->internal_buffer = ctx_buf;

    // 4) Control memcpy length via a local symbolic variable, then assign
    int sym_len;
    memcpy(&sym_len, fuzz_data + (PKT_BUFSZ + CTX_BUFSZ), sizeof(sym_len));
    
    
    avctx->internal_size = sym_len;

    // 5) Call entry (pass-through) into vulnerable slice
    avcodec_receive_packet(avctx, avpkt);
    return 0;
}
