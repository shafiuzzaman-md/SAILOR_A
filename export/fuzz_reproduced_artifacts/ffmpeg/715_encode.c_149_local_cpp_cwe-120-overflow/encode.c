
// harness/encode_spine.c
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// Minimal local types to avoid heavy project headers
#ifndef AV_PKT_DATA_SIZE
#define AV_PKT_DATA_SIZE 64
#endif

typedef struct AVCodecContext {
    // Only keep fields we actually use in this slice
    unsigned char *internal_buffer;
    int internal_size;
} AVCodecContext;

typedef struct AVPacket {
    unsigned char *data;
    int size;
} AVPacket;

// Vulnerable function (neutralized slice): keep only the vulnerable statement
static int encode_receive_packet_internal(AVCodecContext *avctx, AVPacket *avpkt) {
    // Vulnerable memcpy (unchecked length) — universal sink assertion after
    // We intentionally do not check sizes to let KLEE explore overflows
    memcpy(avpkt->data, avctx->internal_buffer, avctx->internal_size);
    klee_assert(0 && "SAILOR_SINK_REACHED");
    return 0;
}

// Entry function: MUST be a simple pass-through without guards
int avcodec_receive_packet(AVCodecContext *avctx, AVPacket *avpkt) {
    encode_receive_packet_internal(avctx, avpkt);
    return 0;
}
