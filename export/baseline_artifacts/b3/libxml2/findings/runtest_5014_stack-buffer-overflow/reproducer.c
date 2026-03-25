#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Minimal type stubs to mirror the libxml2 automata types used in runtest.c */
typedef struct _xmlAutomata xmlAutomata;
typedef struct _xmlAutomataState xmlAutomataState;

struct _xmlAutomata { int dummy; };
struct _xmlAutomataState { int dummy; };

/* Stubbed functions used by runtest.c automataTest() */
static xmlAutomataState *xmlAutomataNewState(xmlAutomata *am) {
    /* Return a non-NULL pointer; allocate so ASan doesn't complain about wild ptr */
    (void)am;
    xmlAutomataState *st = (xmlAutomataState *)malloc(sizeof(xmlAutomataState));
    return st ? st : (xmlAutomataState *)0x1;
}

static void xmlAutomataNewCountTrans(xmlAutomata *am,
                                     xmlAutomataState *from,
                                     xmlAutomataState *to,
                                     const unsigned char *val,
                                     int min, int max, void *data) {
    /* No-op stub; included only to mirror the call in the vulnerable code path */
    (void)am; (void)from; (void)to; (void)val; (void)min; (void)max; (void)data;
}

/* scanNumber implementation matching the behavior used in runtest.c */
static int scanNumber(char **pptr) {
    int ret = 0;
    char *p = *pptr;
    /* Parse consecutive digits only; do not skip leading spaces here */
    while (*p >= '0' && *p <= '9') {
        ret = ret * 10 + (*p - '0');
        p++;
    }
    *pptr = p;
    return ret;
}

/* A minimized reproduction of the 'c ' branch from runtest.c:automataTest() */
static void trigger_overflow_with_c_expr(const char *expr) {
    /* Small stack array to make the overflow obvious to ASan */
    xmlAutomataState *states[8];
    memset(states, 0, sizeof(states));

    /* Non-NULL automaton so the condition (am != NULL) is satisfied */
    xmlAutomata dummy_am; xmlAutomata *am = &dummy_am;

    if (!(am != NULL && expr[0] == 'c' && expr[1] == ' ')) {
        fprintf(stderr, "Expression must start with 'c ' for this reproducer.\n");
        return;
    }

    char *ptr = (char *)&expr[2];
    int from, to;
    int min, max;

    /* from = scanNumber(&ptr); */
    from = scanNumber(&ptr);
    if (*ptr != ' ') {
        fprintf(stderr, "Bad line %s\n", expr);
        return;
    }
    if (states[from] == NULL)
        states[from] = xmlAutomataNewState(am);  /* in-bounds for from=0 */

    ptr++;

    /* to = scanNumber(&ptr); */
    to = scanNumber(&ptr);
    if (*ptr != ' ') {
        fprintf(stderr, "Bad line %s\n", expr);
        return;
    }

    /* VULNERABILITY: No bounds check on 'to' before indexing 'states' */
    if (states[to] == NULL)
        states[to] = xmlAutomataNewState(am);    /* stack OOB write here */

    ptr++;

    /* Continue mirroring the original logic (not necessary for the overflow) */
    min = scanNumber(&ptr);
    if (*ptr != ' ') {
        fprintf(stderr, "Bad line %s\n", expr);
        return;
    }
    ptr++;
    max = scanNumber(&ptr);
    if (*ptr != ' ') {
        fprintf(stderr, "Bad line %s\n", expr);
        return;
    }
    ptr++;

    /* Stub call, just to be faithful to the control flow */
    xmlAutomataNewCountTrans(am, states[from], states[0], (const unsigned char *)ptr, min, max, NULL);

    /* Cleanup some allocated states to avoid leaks when ASan doesn't abort earlier */
    for (size_t i = 0; i < sizeof(states)/sizeof(states[0]); i++) {
        free(states[i]);
    }
}

int main(void) {
    /* Craft an expression that matches the vulnerable 'c ' branch.
       Use from=0 (in-bounds), to=1000000 (way out-of-bounds for states[8]),
       then min=1, max=2 and a trailing value + space as expected by the parser. */
    const char *expr = "c 0 1000000 1 2 X ";

    /* This call will trigger a stack-buffer-overflow in the states[] array
       at the write: states[to] = xmlAutomataNewState(am); */
    trigger_overflow_with_c_expr(expr);

    return 0;
}
