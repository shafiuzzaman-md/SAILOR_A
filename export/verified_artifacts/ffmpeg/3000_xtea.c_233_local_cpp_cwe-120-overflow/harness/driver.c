// NO_HARNESS_TYPES
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>

// Minimal type matching harness
typedef struct AVXTEA {
    uint32_t key[16];
} AVXTEA;

// Prototype of entry function from harness/xtea.c
extern void av_xtea_crypt(AVXTEA *ctx, uint8_t *dst, const uint8_t *src, int count,
                          uint8_t *iv, int decrypt);

int main() {
    // Allocate context
    AVXTEA *ctx = (AVXTEA *)calloc(1, sizeof(AVXTEA));

    // Allocate src and dst buffers for one 8-byte block
    uint8_t *src = (uint8_t *)malloc(8);
    uint8_t *dst = (uint8_t *)malloc(8);
    klee_make_symbolic(src, 8, "src_bytes");
    klee_make_symbolic(dst, 8, "dst_bytes");

    // Undersized IV to trigger overflow in memcpy(iv, dst, 8)
    uint8_t *iv = (uint8_t *)malloc(4);
    klee_make_symbolic(iv, 4, "iv_bytes");

    int count = 1;
    int decrypt = 1;

    av_xtea_crypt(ctx, dst, src, count, iv, decrypt);
    return 0;
}
