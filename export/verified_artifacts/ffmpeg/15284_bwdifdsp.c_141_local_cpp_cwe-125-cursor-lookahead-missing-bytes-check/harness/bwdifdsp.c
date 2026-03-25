#include <stdint.h>
#include <klee/klee.h>

#ifndef FFABS
#define FFABS(a) ((a) >= 0 ? (a) : -(a))
#endif

// Minimal coefficients to satisfy expression dependencies
static const int coef_hf[3] = { 1, 1, 1 };
static const int coef_lf[2] = { 1, 1 };
static const int coef_sp[2] = { 1, 1 };

// Vulnerable function (neutralized to keep only the vulnerable statement path)
void ff_bwdif_filter_line_c(void *dst1, const void *prev1, const void *cur1, const void *next1,
                            int w, int prefs, int mrefs, int prefs2, int mrefs2,
                            int prefs3, int mrefs3, int prefs4, int mrefs4,
                            int parity, int clip_max)
{
    uint8_t *dst   = (uint8_t *)dst1;
    const uint8_t *prev  = (const uint8_t *)prev1;
    const uint8_t *cur   = (const uint8_t *)cur1;
    const uint8_t *next  = (const uint8_t *)next1;
    const uint8_t *prev2 = parity ? prev : cur ;
    const uint8_t *next2 = parity ? cur  : next;
    int interpol, x;

    // Minimal locals used by the vulnerable expression
    int c, e, d = 0, diff = 0, temporal_diff0;

    // Make branch condition choose the path that contains prev2[0] access
    klee_make_symbolic(&c, sizeof(c), "c");
    klee_make_symbolic(&e, sizeof(e), "e");
    temporal_diff0 = 0;
    klee_assume(FFABS(c - e) > temporal_diff0);

    // === Vulnerable statement (verbatim from source macro FILTER_LINE) ===
    interpol = (((coef_hf[0] * (prev2[0] + next2[0])
                    - coef_hf[1] * (prev2[mrefs2] + next2[mrefs2] + prev2[prefs2] + next2[prefs2])
                    + coef_hf[2] * (prev2[mrefs4] + next2[mrefs4] + prev2[prefs4] + next2[prefs4])) >> 2)
                    + coef_lf[0] * (c + e) - coef_lf[1] * (cur[mrefs3] + cur[prefs3])) >> 13;
    // Reachability probe (only triggers if no crash occurred above)
    klee_assert(0 && "SAILOR_SINK_REACHED");
}

// Simple pass-through entry function (MANDATORY pattern)
int entry_func(void *dst1, const void *prev1, const void *cur1, const void *next1,
               int w, int prefs, int mrefs, int prefs2, int mrefs2,
               int prefs3, int mrefs3, int prefs4, int mrefs4,
               int parity, int clip_max) {
    ff_bwdif_filter_line_c(dst1, prev1, cur1, next1,
                           w, prefs, mrefs, prefs2, mrefs2,
                           prefs3, mrefs3, prefs4, mrefs4,
                           parity, clip_max);
    return 0;
}
