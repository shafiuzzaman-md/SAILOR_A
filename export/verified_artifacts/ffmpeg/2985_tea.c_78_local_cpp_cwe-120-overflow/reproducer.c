// Combined reproducer for 2985_tea.c_78_local_cpp_cwe-120-overflow
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: FUNCTION (auto-detected external) */
int FUNCTION() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int av_tea_crypt(AVTEA *ctx, uint8_t *dst, const uint8_t *src, int count, uint8_t *iv, int decrypt);

int main() {
    // Concrete allocations
    AVTEA *ctx = (AVTEA *)calloc(1, sizeof(AVTEA));
    uint8_t *dst = (uint8_t *)malloc(8);
    uint8_t *src = (uint8_t *)malloc(8);
    uint8_t *iv  = (uint8_t *)malloc(4); // deliberately too small to trigger overflow on memcpy(..., 8)

    // Symbolic contents
    klee_make_symbolic(dst, 8, "dst_bytes");
    klee_make_symbolic(src, 8, "src_bytes");
    klee_make_symbolic(iv,  4, "iv_bytes");

    int decrypt = 1; // take the decrypt path with the sink

    // Direct call to entry function
    av_tea_crypt(ctx, dst, src, 1, iv, decrypt);
    return 0;
}
