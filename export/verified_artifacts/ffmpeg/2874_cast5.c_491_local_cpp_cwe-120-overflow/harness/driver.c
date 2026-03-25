#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// entry from harness
int entry_func(AVCAST5* cs, uint8_t* dst, const uint8_t* src, int count, uint8_t *iv, int decrypt);

int main() {
    // Allocate context
    AVCAST5 *cs = (AVCAST5*)calloc(1, sizeof(AVCAST5));

    // Allocate buffers (concrete sizes)
    uint8_t *src = (uint8_t*)malloc(8);
    uint8_t *dst = (uint8_t*)malloc(8);
    // Deliberately small IV to exercise overflow/read issues in the target region
    uint8_t *iv  = (uint8_t*)malloc(4);

    // Make buffer contents symbolic
    klee_make_symbolic(src, 8, "src_buf");
    klee_make_symbolic(dst, 8, "dst_buf");
    klee_make_symbolic(iv, 4, "iv_buf");

    int count = 1;     // single block
    int decrypt = 0;   // take encrypt path with IV

    // Call entry (pass-through to vulnerable function)
    entry_func(cs, dst, src, count, iv, decrypt);
    return 0;
}
