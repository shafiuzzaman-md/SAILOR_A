// Combined reproducer for 23790_timestamp.c_32_local_cpp_cwe-125-cursor-lookahead-missing-bytes-check
// Original harness: driver.c + smart_stubs.c + sliced source

// === smart_stubs.c ===
/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: FFMIN (auto-detected external) */
int FFMIN() { return 0; }

/* PROACTIVE: INFINITY (auto-detected external) */
int INFINITY() { return 0; }

/* PROACTIVE: definition (auto-detected external) */
int definition() { return 0; }

/* PROACTIVE: fabs (auto-detected external) */
int fabs() { return 0; }

/* PROACTIVE: floor (auto-detected external) */
int floor() { return 0; }

/* PROACTIVE: fpclassify (auto-detected external) */
int fpclassify() { return 0; }

/* PROACTIVE: isfinite (auto-detected external) */
int isfinite() { return 0; }

/* PROACTIVE: log10 (auto-detected external) */
int log10() { return 0; }

/* PROACTIVE: snprintf (libc — prevents KLEE concretization) */
int snprintf(char *s, unsigned long n, const char *fmt, ...) { (void)s; (void)n; (void)fmt; if(n>0) s[0]=0; return 0; }

/* PROACTIVE: source_context (auto-detected external) */
int source_context() { return 0; }

// === driver.c ===
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
