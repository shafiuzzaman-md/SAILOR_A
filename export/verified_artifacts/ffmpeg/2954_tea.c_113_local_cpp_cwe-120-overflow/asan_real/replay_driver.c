// Combined reproducer for 2954_tea.c_113_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

// === driver.c ===
// NO_HARNESS_TYPES
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>

// Minimal local type definition matching harness
typedef struct AVTEA {
    uint32_t key[16];
    int rounds;
} AVTEA;

// Prototype for the function defined in harness/tea.c
void av_tea_crypt(AVTEA *ctx, uint8_t *dst, const uint8_t *src, int count,
                  uint8_t *iv, int decrypt);

int main() {
    // Allocate context
    AVTEA *ctx = (AVTEA *)calloc(1, sizeof(AVTEA));

    // Allocate src/dst with enough space (concrete sizes)
    uint8_t *src = (uint8_t *)malloc(16);
    uint8_t *dst = (uint8_t *)malloc(16);

    // Allocate an undersized IV (4 bytes) to trigger overflow at memcpy(iv, dst, 8)
    uint8_t *iv = (uint8_t *)malloc(4);

    // Make buffers symbolic so KLEE explores contents
    klee_make_symbolic(src, 16, "src");
    klee_make_symbolic(dst, 16, "dst");
    klee_make_symbolic(iv, 4, "iv");

    int count = 1;      // one block (8 bytes)
    int decrypt = 0;    // take the encrypt path where iv is used

    av_tea_crypt(ctx, dst, src, count, iv, decrypt);

    return 0;
}
