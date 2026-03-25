#include <stdint.h>
#include <stddef.h>
#include <klee/klee.h>

// Vulnerable function (neutralized to only keep the vulnerable statement)
void ff_acelp_lsp2lpc(int16_t* lp, const int16_t* lsp, int lp_half_order) {
    // Original vulnerable statement from lsp.c:163
    lp[0] = 4096;
    // Universal sink assertion placed AFTER the vulnerable statement
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Simple pass-through entry function (no guards)
int entry_func(int16_t* lp, const int16_t* lsp, int lp_half_order) {
    ff_acelp_lsp2lpc(lp, lsp, lp_half_order);
    return 0;
}
