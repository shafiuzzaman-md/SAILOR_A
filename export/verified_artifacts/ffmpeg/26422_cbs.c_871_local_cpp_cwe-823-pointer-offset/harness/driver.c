#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>
#include <stdint.h>

// entry_func is defined in harness/cbs.c
extern int entry_func(CodedBitstreamFragment *frag, int position);

int main(void) {
    // Concrete allocation of the fragment
    CodedBitstreamFragment *frag = (CodedBitstreamFragment *)calloc(1, sizeof(*frag));

    // Concrete allocation of units buffer with a small capacity
    int cap = 8;
    CodedBitstreamUnit *units = (CodedBitstreamUnit *)malloc(sizeof(*units) * cap);
    // Symbolic content of units to avoid undefined reads/writes
    klee_make_symbolic(units, sizeof(*units) * cap, "units_buf");

    frag->units = units;

    // Symbolic nb_units with reasonable bounds; must be >= 2 so (--nb_units) > 0
    int nb;
    klee_make_symbolic(&nb, sizeof(nb), "nb_units");
    klee_assume(nb >= 2);
    klee_assume(nb <= 32);
    frag->nb_units = nb;

    // Symbolic position; leave unconstrained so KLEE can try negative or large values
    int position;
    klee_make_symbolic(&position, sizeof(position), "position");

    // Direct call to entry (no guards)
    entry_func(frag, position);
    return 0;
}
