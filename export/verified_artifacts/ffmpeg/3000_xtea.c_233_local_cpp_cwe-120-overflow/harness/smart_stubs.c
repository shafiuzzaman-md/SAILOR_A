/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: crypt (auto-detected external) */
int crypt() { return 0; }

/* PROACTIVE: path (auto-detected external) */
int path() { return 0; }

/* PROACTIVE: xtea_crypt (auto-detected external) */
int xtea_crypt() { return 0; }
