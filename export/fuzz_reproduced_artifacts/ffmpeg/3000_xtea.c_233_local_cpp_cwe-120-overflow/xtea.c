#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

// Minimal type needed
typedef struct AVXTEA {
    uint32_t key[16];
} AVXTEA;

// Neutralized vulnerable function: keep signature and the vulnerable path with the exact memcpy line
static void xtea_crypt(AVXTEA *ctx, uint8_t *dst, const uint8_t *src, int count,
                       uint8_t *iv, int decrypt,
                       void (*crypt)(AVXTEA *, uint8_t *, const uint8_t *, int, uint8_t *))
{
    int i;
    // Keep only the CBC-like path (iv != NULL) that leads to the vulnerable memcpy
    while (count--) {
        if (iv) {
            for (i = 0; i < 8; i++)
                dst[i] = src[i] ^ iv[i];
            // Original code calls: crypt(ctx, dst, dst, decrypt, NULL);
            // Neutralized: skip helper call — not needed to reach the sink
            memcpy(iv, dst, 8);  // vulnerable statement — MUST be verbatim
            klee_assert(0 && "SAILOR_SINK_REACHED");
        } else {
            // Unused path in this harness
            ;
        }
        src += 8;
        dst += 8;
    }
}

// Entry function: mandatory pass-through — no guards, just direct call
void av_xtea_crypt(AVXTEA *ctx, uint8_t *dst, const uint8_t *src, int count,
                   uint8_t *iv, int decrypt)
{
    xtea_crypt(ctx, dst, src, count, iv, decrypt, NULL);
}
