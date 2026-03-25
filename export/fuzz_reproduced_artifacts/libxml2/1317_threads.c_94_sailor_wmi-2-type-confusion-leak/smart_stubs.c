/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>

/* PROACTIVE: else (auto-detected external) */
int else_stub() { return 0; }

/* PROACTIVE: pthread_mutex_destroy (auto-detected external) */
int pthread_mutex_destroy() { return 0; }
