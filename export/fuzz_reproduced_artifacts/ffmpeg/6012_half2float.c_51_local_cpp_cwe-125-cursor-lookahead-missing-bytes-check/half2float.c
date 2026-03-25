#include <stdint.h>
#include <stdlib.h>
#include <klee/klee.h>

// Minimal struct capturing the required tables
typedef struct Half2FloatTables {
    uint32_t mantissatable[4096];
    uint32_t exponenttable[64];
    uint32_t offsettable[64];
} Half2FloatTables;

// Entry function must be a simple pass-through (no guards)
int entry_func(Half2FloatTables *t);
void ff_init_half2float_tables(Half2FloatTables *t);

int entry_func(Half2FloatTables *t) {
    ff_init_half2float_tables(t);  // DIRECT call, no guards
    return 0;
}

// Neutralized vulnerable function: keep only the vulnerable statement verbatim
void ff_init_half2float_tables(Half2FloatTables *t)
{
#if !HAVE_FAST_FLOAT16
    // Vulnerable statement (verbatim from half2float.c:51)
    t->exponenttable[0] = 0;
    // Universal sink assertion to mark reachability if no crash occurs
    klee_assert(0 && "SAILOR_SINK_REACHED");
#endif
}
