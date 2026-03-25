#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

// Minimal local type definition
typedef struct AVTEA {
    uint32_t key[16];
    int rounds;
} AVTEA;

// Neutralized helper: ensure dst gets 8 bytes written deterministically
static void tea_crypt_ecb(AVTEA *ctx, uint8_t *dst, const uint8_t *src,
                          int decrypt, uint8_t *iv)
{
    (void)ctx; (void)decrypt; (void)iv;
    // Simple copy to produce 8 output bytes
    for (int i = 0; i < 8; i++)
        dst[i] = src[i];
}

// Vulnerable function slice — KEEP the exact vulnerable statement text
void av_tea_crypt(AVTEA *ctx, uint8_t *dst, const uint8_t *src, int count,
                  uint8_t *iv, int decrypt)
{
    int i;

    // Neutralized: focus only on the encrypt path where iv != NULL
    while (count--) {
        if (iv) {
            for (i = 0; i < 8; i++)
                dst[i] = src[i] ^ iv[i];
            tea_crypt_ecb(ctx, dst, dst, decrypt, NULL);
            memcpy(iv, dst, 8);
            // Universal sink assertion after the vulnerable statement
            klee_assert(0 && "SAILOR_SINK_REACHED");
        } else {
            tea_crypt_ecb(ctx, dst, src, decrypt, NULL);
        }
        src   += 8;
        dst   += 8;
    }
}
