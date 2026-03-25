/* Smart stubs — auto-generated from path + vulnerability analysis */
/* Symbolic stubs model the environment: KLEE explores return values */
/* that both REACH the sink AND TRIGGER the vulnerability */
#include <stdlib.h>
#include <string.h>
// klee removed

/* PROACTIVE: functions (auto-detected external) */
int functions() { return 0; }

/* PROACTIVE: strdup (auto-detected external) */
#include <stdlib.h>
#include <string.h>
char *strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}


/* PROACTIVE: types (auto-detected external) */
int types() { return 0; }
