#include "harness_types.h"
#include <klee/klee.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef AV_TS_MAX_STRING_SIZE
#define AV_TS_MAX_STRING_SIZE 1
#endif
#ifndef AV_NOPTS_VALUE
#define AV_NOPTS_VALUE (-9223372036854775807LL - 1)
#endif

int entry_func(char *buf, int64_t ts, struct AVRational tb);

int main() {
    // Concrete-sized buffer as required (size = AV_TS_MAX_STRING_SIZE = 1)
    char buf[AV_TS_MAX_STRING_SIZE];
    klee_make_symbolic(buf, sizeof(buf), "buf");

    int64_t ts;
    klee_make_symbolic(&ts, sizeof(ts), "ts");
    klee_assume(ts != AV_NOPTS_VALUE);  // force else-branch

    struct AVRational tb;
    klee_make_symbolic(&tb, sizeof(tb), "tb");
    // Keep denominator unconstrained; av_q2d guards div-by-zero internally

    // Direct call into entry (pass-through to vulnerable function)
    entry_func(buf, ts, tb);
    return 0;
}
