#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Minimal stub types to mimic libxml2 automata API used by runtest.c */
typedef struct _xmlAutomata {
    int dummy;
} xmlAutomata;

typedef struct _xmlAutomataState {
    int dummy;
} xmlAutomataState;

typedef xmlAutomata* xmlAutomataPtr;
typedef xmlAutomataState* xmlAutomataStatePtr;

/* Stubs for the API functions referenced by the vulnerable code. */
static xmlAutomataStatePtr xmlAutomataNewState(xmlAutomataPtr am) {
    (void)am;
    /* Return some non-NULL state; not actually used in this reproducer. */
    static xmlAutomataState dummy;
    return &dummy;
}

static void xmlAutomataSetFinalState(xmlAutomataPtr am, xmlAutomataStatePtr state) {
    /* No-op stub: just consume the arguments. */
    (void)am;
    (void)state;
}

/* Helper copied in spirit from runtest.c: parses an integer from a string pointer */
static int scanNumber(char **pptr) {
    char *p = *pptr;
    int ret = 0;
    /* Skip leading spaces */
    while (*p == ' ') p++;
    /* Parse decimal digits */
    while (isdigit((unsigned char)*p)) {
        ret = ret * 10 + (*p - '0');
        p++;
    }
    *pptr = p;
    return ret;
}

/* Reimplementation of the vulnerable 'f ' branch from runtest.c::automataTest. */
static void automataTest_trigger_f_line(const char *expr) {
    /* In the real code, 'states' is an array of 1000 pointers. */
    xmlAutomataStatePtr states[1000];
    /* Initialize to NULL as done by the test harness before use. */
    memset(states, 0, sizeof(states));

    /* 'am' must be non-NULL to enter the vulnerable branch. */
    xmlAutomataPtr am = (xmlAutomataPtr)0x1; /* any non-NULL value */

    if ((am != NULL) && (expr[0] == 'f') && (expr[1] == ' ')) {
        char *ptr = (char *)&expr[2];
        int state;

        state = scanNumber(&ptr);
        /*
         * Vulnerability: no bounds check on 'state'. If state >= 1000,
         * the following line reads past the end of the 'states' array.
         */
        if (states[state] == NULL) { /* Out-of-bounds read when state >= 1000 */
            fprintf(stderr, "Bad state %d : %s\n", state, expr);
            /* In the original code, this would 'break' out of a loop. We continue
             * here to also exercise the subsequent use for completeness. */
        }
        /* Subsequent use may pass an invalid pointer if the OOB read yielded non-NULL. */
        xmlAutomataSetFinalState(am, states[state]);
    }
}

int main(void) {
    /* Craft input that triggers the OOB read: index 1000 is just past the 0..999 range. */
    const char *line = "f 1000";  /* Reachable via 'f ' lines as in the vulnerable code */
    automataTest_trigger_f_line(line);
    return 0;
}
