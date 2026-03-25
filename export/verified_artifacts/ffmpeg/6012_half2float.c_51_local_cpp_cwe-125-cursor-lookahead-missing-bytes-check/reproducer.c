// Combined reproducer for 6012_half2float.c_51_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: statement (auto-detected external) */
int statement() { return 0; }

/* PROACTIVE: through (auto-detected external) */
int through() { return 0; }

// === driver.c ===
#include "harness_types.h"
#include <klee/klee.h>
#include <stdlib.h>
#include <stdint.h>

int entry_func(Half2FloatTables *t);

int main() {
    // Intentionally undersized allocation to trigger OOB when writing exponenttable[0]
    size_t tiny = 8; // concrete small size
    void *raw = calloc(1, tiny);
    if (!raw) return 0;
    // make memory symbolic to explore variations (optional)
    klee_make_symbolic(raw, tiny, "tiny_tables_mem");

    Half2FloatTables *t = (Half2FloatTables *)raw;
    entry_func(t);
    return 0;
}
