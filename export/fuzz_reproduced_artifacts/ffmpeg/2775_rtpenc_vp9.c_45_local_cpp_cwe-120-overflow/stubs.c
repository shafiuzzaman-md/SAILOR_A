#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>

void ff_rtp_send_data(AVFormatContext *ctx, const uint8_t *buf, int len, int last) {
    // No-op stub. Keep path simple.
    (void)ctx; (void)buf; (void)len; (void)last;
}
