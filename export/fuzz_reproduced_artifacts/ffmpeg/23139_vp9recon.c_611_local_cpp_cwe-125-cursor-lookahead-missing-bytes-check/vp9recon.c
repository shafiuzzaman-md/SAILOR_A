#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

#ifndef av_always_inline
#define av_always_inline
#endif

// Minimal local stand-in for the VP9 tables used by the vulnerable code
// Dimensions chosen small; contents irrelevant for triggering OOB with symbolic index
static const uint8_t ff_vp9_bwh_tab[2][4][2] = {
    { {1,1}, {2,2}, {3,3}, {4,4} },
    { {1,1}, {2,2}, {3,3}, {4,4} }
};

// Minimal struct definitions with only fields used by the sliced code
typedef struct VP9Block {
    int bs;   // block size index used to index ff_vp9_bwh_tab
    int tx;   // transform size used in shifts
} VP9Block;

typedef struct VP9Context {
    int _unused; // placeholder; not used in this slice
} VP9Context;

typedef struct VP9TileData {
    VP9Context *s; // not used by sliced statements but kept for signature parity
    VP9Block   *b; // provides bs and tx
    int row, col;  // unused in slice
} VP9TileData;

// Entry function: DIRECT pass-through to vulnerable function (no guards)
static av_always_inline void inter_recon(VP9TileData *td, int bytesperpixel);
void ff_vp9_inter_recon_8bpp(VP9TileData *td) {
    inter_recon(td, 1);
}

// Vulnerable function slice: keep only the target statements and the sink
static av_always_inline void inter_recon(VP9TileData *td, int bytesperpixel) {
    VP9Block *b = td->b;
    // Vulnerable statements copied verbatim from source (vp9recon.c:611-612)
    int w4 = ff_vp9_bwh_tab[1][b->bs][0] << 1, step1d = 1 << b->tx, n;
    int h4 = ff_vp9_bwh_tab[1][b->bs][1] << 1, x, y, step = 1 << (b->tx * 2);

    // Universal sink assertion: fires if site is reachable without crash
    klee_assert(0 && "SAILOR_SINK_REACHED");
}
