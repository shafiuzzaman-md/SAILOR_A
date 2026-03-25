#include <stddef.h>
// Combined reproducer for 26422_cbs.c_871_local_cpp_cwe-823-pointer-offset
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
/* PROACTIVE: av_assert0 (auto-detected external) */
int av_assert0() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: neutralized (auto-detected external) */
int neutralized() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <stdlib.h>
#include <stdint.h>

// entry_func is defined in harness/cbs.c
extern int entry_func(CodedBitstreamFragment *frag, int position);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 64) return 0;
    // Concrete allocation of the fragment
    CodedBitstreamFragment *frag = (CodedBitstreamFragment *)calloc(1, sizeof(*frag));

    // Concrete allocation of units buffer with a small capacity
    int cap = 8;
    CodedBitstreamUnit *units = (CodedBitstreamUnit *)malloc(sizeof(*units) * cap);
    // Symbolic content of units to avoid undefined reads/writes
    memcpy(units, fuzz_data + (0), sizeof(*units) * cap);

    frag->units = units;

    // Symbolic nb_units with reasonable bounds; must be >= 2 so (--nb_units) > 0
    int nb;
    memcpy(&nb, fuzz_data + (sizeof(*units) * cap), sizeof(nb));
    
    
    frag->nb_units = nb;

    // Symbolic position; leave unconstrained so KLEE can try negative or large values
    int position;
    memcpy(&position, fuzz_data + (sizeof(*units) * cap + sizeof(nb)), sizeof(position));

    // Direct call to entry (no guards)
    entry_func(frag, position);
    return 0;
}
