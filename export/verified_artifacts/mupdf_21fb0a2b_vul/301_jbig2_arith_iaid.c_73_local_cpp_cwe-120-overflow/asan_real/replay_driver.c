#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>

// Forward decl for entry/vulnerable function
Jbig2ArithIaidCtx * jbig2_arith_iaid_ctx_new(Jbig2Ctx *ctx, uint8_t SBSYMCODELEN);

int main() {
    // Allocate context and allocator concretely
    Jbig2Ctx *ctx = (Jbig2Ctx *)calloc(1, sizeof(Jbig2Ctx));
    Jbig2Allocator *alloc = (Jbig2Allocator *)calloc(1, sizeof(Jbig2Allocator));
    ctx->allocator = alloc;

    // Symbolic SBSYMCODELEN with constraints to reach sink and trigger wrap
    uint8_t s = 0;
    { static const unsigned char SBSYMCODELEN_data[] = {0x3f}; memcpy(&s, SBSYMCODELEN_data, (sizeof(s) < sizeof(SBSYMCODELEN_data)) ? sizeof(s) : sizeof(SBSYMCODELEN_data)); };
    // Must be < 64 to pass early guard, and == 63 to cause size_t wrap in bytes = sizeof(Jbig2ArithCx) * (1ULL<<s)
    /* klee_assume removed */
    /* klee_assume removed */

    (void)jbig2_arith_iaid_ctx_new(ctx, s);
    return 0;
}
