#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>

// Declarations from harness
Jbig2SymbolDict * jbig2_sd_cat(Jbig2Ctx *ctx, uint32_t n_dicts, Jbig2SymbolDict **dicts);

int main() {
    // Allocate context
    Jbig2Ctx *ctx = (Jbig2Ctx *)calloc(1, sizeof(Jbig2Ctx));
    if (!ctx) return 0;

    // n_dicts is used as n_symbols in jbig2_sd_new via neutralized entry
    uint32_t n_dicts;
    { static const unsigned char n_dicts_data[] = {0x10, 0x00, 0x00, 0x80}; memcpy(&n_dicts, n_dicts_data, (sizeof(n_dicts) < sizeof(n_dicts_data)) ? sizeof(n_dicts) : sizeof(n_dicts_data)); };

    // Constrain to be non-zero
    /* klee_assume removed */

    // Force 32-bit overflow in size computation: (uint64)n * sizeof(Jbig2Image*) > 0xFFFFFFFF
    uint64_t prod = (uint64_t)n_dicts * (uint64_t)sizeof(Jbig2Image *);
    /* klee_assume removed */

    // Also ensure the truncated 32-bit size used by allocator is non-zero, so malloc succeeds
    uint32_t truncated = (uint32_t)prod;
    /* klee_assume removed */

    // dicts is unused in neutralized entry; pass NULL
    Jbig2SymbolDict *res = jbig2_sd_cat(ctx, n_dicts, NULL);
    (void)res;
    return 0;
}
