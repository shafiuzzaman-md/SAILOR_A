/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

/* PROACTIVE: FFMIN (auto-detected external) */
int FFMIN() { return 0; }

/* PROACTIVE: INFINITY (auto-detected external) */
int INFINITY() { return 0; }

/* PROACTIVE: definition (auto-detected external) */
int definition() { return 0; }

/* PROACTIVE: fabs (auto-detected external) */
int fabs() { return 0; }

/* PROACTIVE: floor (auto-detected external) */
int floor() { return 0; }

/* PROACTIVE: fpclassify (auto-detected external) */
int fpclassify() { return 0; }

/* PROACTIVE: isfinite (auto-detected external) */
int isfinite() { return 0; }

/* PROACTIVE: log10 (auto-detected external) */
int log10() { return 0; }

/* PROACTIVE: snprintf (libc — prevents KLEE concretization) */
int snprintf(char *s, unsigned long n, const char *fmt, ...) { (void)s; (void)n; (void)fmt; if(n>0) s[0]=0; return 0; }

/* PROACTIVE: source_context (auto-detected external) */
int source_context() { return 0; }
