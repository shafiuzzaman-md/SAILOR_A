/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// klee removed

/* PROACTIVE: clone (auto-detected external) */
int clone() { return 0; }

/* PROACTIVE: entry (auto-detected external) */
int entry() { return 0; }

/* PROACTIVE: strdup (auto-detected external) */
#include <string.h>
#include <stdlib.h>
char *strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *p = (char *)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}
