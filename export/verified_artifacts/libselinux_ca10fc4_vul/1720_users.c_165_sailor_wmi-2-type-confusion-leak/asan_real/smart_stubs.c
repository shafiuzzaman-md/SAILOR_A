/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// klee removed

/* PROACTIVE: fields (auto-detected external) */
int fields() { return 0; }

/* PROACTIVE: function (auto-detected external) */
int function() { return 0; }

/* PROACTIVE: msg_callback (auto-detected external) */
int msg_callback() { return 0; }

/* PROACTIVE: role (auto-detected external) */
int role() { return 0; }

/* PROACTIVE: snprintf (libc — prevents KLEE concretization) */
int snprintf(char *s, unsigned long n, const char *fmt, ...) { (void)s; (void)n; (void)fmt; if(n>0) s[0]=0; return 0; }
