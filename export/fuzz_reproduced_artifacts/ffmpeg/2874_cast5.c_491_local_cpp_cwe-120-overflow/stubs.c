#include "harness_types.h"
#include <stdint.h>
#include <string.h>

void encipher(AVCAST5* cs, uint8_t* dst, const uint8_t* src) {
    (void)cs;
    for (int i = 0; i < 8; i++) dst[i] = src[i];
}

void decipher(AVCAST5* cs, uint8_t* dst, const uint8_t* src, uint8_t *iv) {
    (void)cs; (void)iv;
    for (int i = 0; i < 8; i++) dst[i] = src[i];
}
