/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// klee removed

/* PROACTIVE: fields (auto-detected external) */
int fields() { return 0; }

/* PROACTIVE: msg_callback (auto-detected external) */
int msg_callback() { return 0; }

/* PROACTIVE: strdup (auto-detected external) */
char *strdup(const char *s) { (void)s; return NULL; }
