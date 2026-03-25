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
