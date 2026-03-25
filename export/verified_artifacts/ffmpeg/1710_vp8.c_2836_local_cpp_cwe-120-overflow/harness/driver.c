// NO_HARNESS_TYPES
#include <stdlib.h>
#include <klee/klee.h>

#ifndef VP7_MVC_SIZE
#define VP7_MVC_SIZE 17
#endif
#ifndef VP8_MVC_SIZE
#define VP8_MVC_SIZE 19
#endif

// Minimal matching types for entry prototype
typedef struct AVCodecContext {
    void *priv_data;
} AVCodecContext;

// Entry from harness
int ff_vp8_decode_frame(AVCodecContext *avctx, void *frame,
                        int *got_frame, void *avpkt);

int main() {
    AVCodecContext *avctx = (AVCodecContext *)calloc(1, sizeof(AVCodecContext));

    // Intentionally small allocation to trigger overflow in memcpy inside vp78_decode_frame
    const size_t SMALL_PRIV = 80; // smaller than memcpy size (4 * 64 = 256)
    void *priv = calloc(1, SMALL_PRIV);
    klee_make_symbolic(priv, SMALL_PRIV, "vp8_ctx_small");
    avctx->priv_data = priv;

    void *frame = malloc(1);
    void *pkt = malloc(1);
    int got = 0;

    ff_vp8_decode_frame(avctx, frame, &got, pkt);
    return 0;
}
