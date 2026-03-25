#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <klee/klee.h>

// Type from project (provided via harness_types as well)
typedef struct AVCAST5 {
    uint32_t Km[17];
    uint32_t Kr[17];
    int rounds;
} AVCAST5;

// Forward declarations (implemented in stubs.c)
void encipher(AVCAST5* cs, uint8_t* dst, const uint8_t* src);
void decipher(AVCAST5* cs, uint8_t* dst, const uint8_t* src, uint8_t *iv);

// VULNERABLE FUNCTION (keep verbatim sink line)
void av_cast5_crypt2(AVCAST5* cs, uint8_t* dst, const uint8_t* src, int count, uint8_t *iv, int decrypt)
{
    int i;
    while (count--) {
        if (decrypt) {
            decipher(cs, dst, src, iv);
        } else {
            if (iv) {
                for (i = 0; i < 8; i++)
                    dst[i] = src[i] ^ iv[i];
                encipher(cs, dst, dst);
                memcpy(iv, dst, 8);
                klee_assert(0 && "SAILOR_SINK_REACHED");
            } else {
                encipher(cs, dst, src);
            }
        }
        src = src + 8;
        dst = dst + 8;
    }
}

// ENTRY: mandatory simple pass-through
int entry_func(AVCAST5* cs, uint8_t* dst, const uint8_t* src, int count, uint8_t *iv, int decrypt) {
    av_cast5_crypt2(cs, dst, src, count, iv, decrypt);
    return 0;
}
