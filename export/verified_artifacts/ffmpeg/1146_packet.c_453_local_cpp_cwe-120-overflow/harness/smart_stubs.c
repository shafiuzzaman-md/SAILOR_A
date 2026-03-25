/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: AVERROR (auto-detected external) */
int AVERROR() { return 0; }

/* PROACTIVE: av_assert1 (auto-detected external) */
int av_assert1() { return 0; }
