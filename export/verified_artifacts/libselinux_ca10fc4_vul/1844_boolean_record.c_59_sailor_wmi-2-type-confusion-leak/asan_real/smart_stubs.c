/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// klee removed

/* PROACTIVE: completeness (auto-detected external) */
int completeness() { return 0; }

/* PROACTIVE: definitions (auto-detected external) */
int definitions() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: ins (auto-detected external) */
int ins() { return 0; }

/* PROACTIVE: path (auto-detected external) */
int path() { return 0; }

/* PROACTIVE: printf (libc — prevents KLEE concretization) */
int printf(const char *fmt, ...) { (void)fmt; return 0; }
