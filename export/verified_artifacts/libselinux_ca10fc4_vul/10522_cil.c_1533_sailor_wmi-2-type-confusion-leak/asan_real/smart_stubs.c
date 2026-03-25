/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// klee removed

/* PROACTIVE: cil_list_for_each (auto-detected external) */
int cil_list_for_each() { return 0; }

/* PROACTIVE: cil_userprefix (auto-detected external) */
int cil_userprefix() { return 0; }

/* PROACTIVE: references (auto-detected external) */
int references() { return 0; }

/* PROACTIVE: snprintf (libc — prevents KLEE concretization) */
int snprintf(char *s, unsigned long n, const char *fmt, ...) { (void)s; (void)n; (void)fmt; if(n>0) s[0]=0; return 0; }

/* PROACTIVE: strlen (libc — prevents KLEE concretization) */
unsigned long strlen(const char *s) { (void)s; return 0; }

/* PROACTIVE: test (auto-detected external) */
int test() { return 0; }
