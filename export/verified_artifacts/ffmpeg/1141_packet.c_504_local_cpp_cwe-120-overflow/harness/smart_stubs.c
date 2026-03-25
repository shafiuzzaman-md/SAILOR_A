/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: ENOMEM (auto-detected external) */
int ENOMEM() { return 0; }

/* PROACTIVE: FUNCTION (auto-detected external) */
int FUNCTION() { return 0; }

/* PROACTIVE: av_assert1 (auto-detected external) */
int av_assert1() { return 0; }

/* PROACTIVE: probe (auto-detected external) */
int probe() { return 0; }

/* PROACTIVE: through (auto-detected external) */
int through() { return 0; }
