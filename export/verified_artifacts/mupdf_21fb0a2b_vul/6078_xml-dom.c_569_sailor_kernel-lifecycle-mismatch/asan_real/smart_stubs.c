/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// klee removed

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: keep (auto-detected external) */
int keep() { return 0; }

/* PROACTIVE: models (auto-detected external) */
int models() { return 0; }

/* PROACTIVE: strcmp (libc — prevents KLEE concretization) */
int strcmp(const char *a, const char *b) { (void)a; (void)b; return 1; }

/* PROACTIVE: wrapper (auto-detected external) */
int wrapper() { return 0; }
