#include <stddef.h>
// Combined reproducer for 3000_xtea.c_233_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: crypt (auto-detected external) */
int crypt() { return 0; }

/* PROACTIVE: path (auto-detected external) */
int path() { return 0; }

/* PROACTIVE: xtea_crypt (auto-detected external) */
int xtea_crypt() { return 0; }

// === driver.c ===
// NO_HARNESS_TYPES
#include <stdint.h>
#include <stdlib.h>

// Minimal type matching harness
typedef struct AVXTEA {
    uint32_t key[16];
} AVXTEA;

// Prototype of entry function from harness/xtea.c
extern void av_xtea_crypt(AVXTEA *ctx, uint8_t *dst, const uint8_t *src, int count,
                          uint8_t *iv, int decrypt);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Allocate context
    AVXTEA *ctx = (AVXTEA *)calloc(1, sizeof(AVXTEA));

    // Allocate src and dst buffers for one 8-byte block
    uint8_t *src = (uint8_t *)malloc(8);
    uint8_t *dst = (uint8_t *)malloc(8);
    memcpy(src, fuzz_data + (0), 8);
    memcpy(dst, fuzz_data + (8), 8);

    // Undersized IV to trigger overflow in memcpy(iv, dst, 8)
    uint8_t *iv = (uint8_t *)malloc(4);
    memcpy(iv, fuzz_data + (8 + 8), 4);

    int count = 1;
    int decrypt = 1;

    av_xtea_crypt(ctx, dst, src, count, iv, decrypt);
    return 0;
}
