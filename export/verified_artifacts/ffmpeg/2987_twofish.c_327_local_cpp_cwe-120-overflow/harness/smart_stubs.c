/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: twofish_decrypt (auto-detected external) */
int twofish_decrypt() { return 0; }

/* PROACTIVE: twofish_encrypt (auto-detected external) */
int twofish_encrypt() { return 0; }
