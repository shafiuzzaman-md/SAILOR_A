#include "harness_types.h"
#include <stddef.h>

// No-op transform stub matching av_tx_fn signature
void fake_av_tx(AVTXContext *ctx, float *dst, void *src, ptrdiff_t stride) {
    (void)ctx; (void)dst; (void)src; (void)stride;
}
