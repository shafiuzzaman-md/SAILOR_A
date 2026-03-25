/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// klee removed

/* PROACTIVE: __builtin_trap (auto-detected external) */
/* removed conflicting __builtin_trap stub; use compiler builtin */

/* PROACTIVE: memcmp (libc — prevents KLEE concretization) */
int memcmp(const void *a, const void *b, unsigned long n) { (void)a; (void)b; (void)n; return 1; }

/* PROACTIVE: slice (auto-detected external) */
int slice() { return 0; }
