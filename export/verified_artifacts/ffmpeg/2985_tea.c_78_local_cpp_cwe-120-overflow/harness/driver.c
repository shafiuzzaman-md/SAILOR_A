#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int av_tea_crypt(AVTEA *ctx, uint8_t *dst, const uint8_t *src, int count, uint8_t *iv, int decrypt);

int main() {
    // Concrete allocations
    AVTEA *ctx = (AVTEA *)calloc(1, sizeof(AVTEA));
    uint8_t *dst = (uint8_t *)malloc(8);
    uint8_t *src = (uint8_t *)malloc(8);
    uint8_t *iv  = (uint8_t *)malloc(4); // deliberately too small to trigger overflow on memcpy(..., 8)

    // Symbolic contents
    klee_make_symbolic(dst, 8, "dst_bytes");
    klee_make_symbolic(src, 8, "src_bytes");
    klee_make_symbolic(iv,  4, "iv_bytes");

    int decrypt = 1; // take the decrypt path with the sink

    // Direct call to entry function
    av_tea_crypt(ctx, dst, src, 1, iv, decrypt);
    return 0;
}
