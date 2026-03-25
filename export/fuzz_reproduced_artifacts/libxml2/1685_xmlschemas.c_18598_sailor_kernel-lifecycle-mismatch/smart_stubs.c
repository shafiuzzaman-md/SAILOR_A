/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>

/* PROACTIVE: access (auto-detected external) */
int access() { return 0; }

/* PROACTIVE: callee (auto-detected external) */
int callee() { return 0; }
