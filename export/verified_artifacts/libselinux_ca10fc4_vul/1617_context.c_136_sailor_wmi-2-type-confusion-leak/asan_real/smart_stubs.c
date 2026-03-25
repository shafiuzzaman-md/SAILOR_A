/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// klee removed

/* PROACTIVE: stpcpy (auto-detected external) */
char *stpcpy(char *dest, const char *src) {
    char *d = dest;
    const char *s = src;
    while ((*d = *s) != '\0') { ++d; ++s; }
    return d;
}


/* PROACTIVE: strlen (libc — prevents KLEE concretization) */
unsigned long strlen(const char *s) { (void)s; return 0; }
