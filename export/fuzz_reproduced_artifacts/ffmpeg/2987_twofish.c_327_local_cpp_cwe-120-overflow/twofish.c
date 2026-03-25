#include <stdint.h>
#include <string.h>
#include <klee/klee.h>

// Minimal type needed by the vulnerable function
typedef struct AVTWOFISH {
    uint32_t K[40];
    uint32_t S[4];
    int ksize;
    uint32_t MDS1[256];
    uint32_t MDS2[256];
    uint32_t MDS3[256];
    uint32_t MDS4[256];
} AVTWOFISH;

// NEUTRALIZED vulnerable function: keep signature and sink path only
void av_twofish_crypt(AVTWOFISH *cs, uint8_t *dst, const uint8_t *src, int count, uint8_t *iv, int decrypt)
{
    int i;  // keep local as in original signature/body
    if (1) {  // neutralized loop
        if (decrypt) {
            // removed: twofish_decrypt(cs, dst, src, iv);
        } else {
            if (iv) {
                // removed: for (i = 0; i < 16; i++) dst[i] = src[i] ^ iv[i];
                // removed: twofish_encrypt(cs, dst, dst);
                memcpy(iv, dst, 16);  // vulnerable statement — EXACT TEXT
                klee_assert(0 && "SAILOR_SINK_REACHED");
            } else {
                // removed: twofish_encrypt(cs, dst, src);
            }
        }
        // removed: src += 16; dst += 16;
    }
}

// ENTRY — mandatory simple pass-through wrapper
int entry_func(AVTWOFISH *cs, uint8_t *dst, const uint8_t *src, int count, uint8_t *iv, int decrypt) {
    av_twofish_crypt(cs, dst, src, count, iv, decrypt);  // DIRECT call, no guards
    return 0;
}
