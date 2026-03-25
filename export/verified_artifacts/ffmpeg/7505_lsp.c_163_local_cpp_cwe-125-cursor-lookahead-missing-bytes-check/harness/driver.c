#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int entry_func(int16_t* lp, const int16_t* lsp, int lp_half_order);

int main() {
    // Allocate a 1-byte buffer so that writing lp[0] (2 bytes) overflows
    char *small = (char *)malloc(1);
    klee_assert(small != NULL);
    // Make the single byte symbolic to avoid concrete folding
    klee_make_symbolic(small, 1, "lp_byte");
    int16_t *lp = (int16_t *)small;

    // lsp is unused in our neutralized harness but provide a valid allocation
    int16_t *lsp = (int16_t *)malloc(2 * sizeof(int16_t));
    klee_assert(lsp != NULL);
    klee_make_symbolic(lsp, 2 * sizeof(int16_t), "lsp_buf");

    int lp_half_order = 1; // value irrelevant in neutralized harness

    // Directly call entry
    entry_func(lp, lsp, lp_half_order);
    return 0;
}
