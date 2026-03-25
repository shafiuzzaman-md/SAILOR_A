/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// klee removed

/* PROACTIVE: defs (auto-detected external) */
int defs() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: source (auto-detected external) */
int source() { return 0; }

/* PROACTIVE: strcmp (libc — prevents KLEE concretization) */
int strcmp(const char *a, const char *b) { (void)a; (void)b; return 1; }
