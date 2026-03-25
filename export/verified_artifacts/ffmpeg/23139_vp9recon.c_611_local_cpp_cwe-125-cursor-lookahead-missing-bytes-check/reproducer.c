// Combined reproducer for 23139_vp9recon.c_611_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: source (auto-detected external) */
int source() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdlib.h>
#include <klee/klee.h>

// Entry function from the harness
void ff_vp9_inter_recon_8bpp(VP9TileData *td);

int main() {
    // Allocate concrete objects
    VP9TileData *td = (VP9TileData *)calloc(1, sizeof(VP9TileData));
    VP9Block *b = (VP9Block *)calloc(1, sizeof(VP9Block));
    VP9Context *s = (VP9Context *)calloc(1, sizeof(VP9Context));

    // Wire pointers
    td->b = b;
    td->s = s;
    td->row = 0;
    td->col = 0;

    // Make locals symbolic, then assign to fields
    int bs_sym, tx_sym;
    klee_make_symbolic(&bs_sym, sizeof(bs_sym), "bs");
    klee_make_symbolic(&tx_sym, sizeof(tx_sym), "tx");
    // Do NOT constrain bs_sym — allow OOB index to trigger the bug
    // Constrain tx to avoid undefined huge shifts
    klee_assume(tx_sym >= 0);
    klee_assume(tx_sym <= 3);
    b->bs = bs_sym;
    b->tx = tx_sym;

    // Call entry (direct pass-through to vulnerable function)
    ff_vp9_inter_recon_8bpp(td);
    return 0;
}
