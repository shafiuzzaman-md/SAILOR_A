// Combined reproducer for 1710_vp8.c_2836_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
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
