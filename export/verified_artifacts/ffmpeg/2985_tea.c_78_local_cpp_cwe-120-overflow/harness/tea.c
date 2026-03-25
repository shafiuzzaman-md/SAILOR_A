/* Minimal harness for tea.c:78 memcpy(iv, src, 8) overflow */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#ifndef KLEE_SAILOR_LOCAL_DEFS
#define KLEE_SAILOR_LOCAL_DEFS 1
#endif

// Minimal stand-in for FFmpeg's AVTEA
typedef struct AVTEA {
    uint32_t key[4];
    int rounds;
} AVTEA;

// Minimal big-endian helpers compatible with FFmpeg style
static inline uint32_t AV_RB32(const void *p) {
    const uint8_t *b = (const uint8_t *)p;
    return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | (uint32_t)b[3];
}
static inline void AV_WB32(void *p, uint32_t v) {
    uint8_t *b = (uint8_t *)p;
    b[0] = (uint8_t)(v >> 24);
    b[1] = (uint8_t)(v >> 16);
    b[2] = (uint8_t)(v >> 8);
    b[3] = (uint8_t)(v);
}

// VULNERABLE FUNCTION (neutralized to keep only the path with the sink)
// Keep signature similar to FFmpeg internal block crypt routine
void tea_crypt_block(AVTEA *ctx, uint8_t *dst, const uint8_t *src, int rounds, uint8_t *iv, int decrypt) {
    // Minimal variables consistent with source context
    uint32_t v0 = AV_RB32(src);
    uint32_t v1 = AV_RB32(src + 4);

    if (decrypt) {
        if (iv) {
            /* neutralized to bypass non-target OOB read */ /* v0 ^= AV_RB32(iv); */
            /* neutralized to bypass non-target OOB read */ /* v1 ^= AV_RB32(iv + 4); */
            // VULNERABLE STATEMENT — must be verbatim
            memcpy(iv, src, 8);
            // UNIVERSAL SINK ASSERTION placed AFTER the vulnerable statement
            klee_assert(0 && "SAILOR_SINK_REACHED");
        }
    }

    AV_WB32(dst, v0);
    AV_WB32(dst + 4, v1);
}

// ENTRY FUNCTION — MUST directly call the vulnerable function (no guards)
// Signature modeled on FFmpeg public API av_tea_crypt(..., count, iv, decrypt)
int av_tea_crypt(AVTEA *ctx, uint8_t *dst, const uint8_t *src, int count, uint8_t *iv, int decrypt) {
    // Direct pass-through: one block is enough to reach the sink
    tea_crypt_block(ctx, dst, src, 64, iv, decrypt);
    return 0;
}
