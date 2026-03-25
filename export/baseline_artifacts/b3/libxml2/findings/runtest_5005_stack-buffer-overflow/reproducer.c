#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Stubs and minimal type aliases to mimic libxml2 automata API used in runtest.c */
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

typedef struct _xmlAutomata { int dummy; } xmlAutomata;
typedef struct _xmlAutomata *xmlAutomataPtr;

typedef struct _xmlAutomataState { int id; } xmlAutomataState;
typedef struct _xmlAutomataState *xmlAutomataStatePtr;

/* Stub implementations just to make the code run */
static xmlAutomataStatePtr xmlAutomataNewState(xmlAutomataPtr am) {
    (void)am;
    xmlAutomataStatePtr s = (xmlAutomataStatePtr)malloc(sizeof(xmlAutomataState));
    if (s) s->id = 42;
    return s;
}

static void xmlAutomataNewCountTrans(xmlAutomataPtr am,
                                     xmlAutomataStatePtr from,
                                     xmlAutomataStatePtr to,
                                     const xmlChar *val,
                                     int min, int max, void *data) {
    /* No-op; just a stub so the code path is executed */
    (void)am; (void)from; (void)to; (void)val; (void)min; (void)max; (void)data;
}

/* Minimal scanNumber matching the usage pattern in runtest.c */
static int scanNumber(char **pptr) {
    char *p = *pptr;
    int sign = 1;
    long val = 0;

    if (*p == '+') p++;
    else if (*p == '-') { sign = -1; p++; }

    if (!isdigit((unsigned char)*p)) {
        *pptr = p; /* leave pointer as-is on failure */
        return 0;
    }

    while (isdigit((unsigned char)*p)) {
        val = val * 10 + (*p - '0');
        p++;
    }
    *pptr = p; /* points at first non-digit, which runtest.c expects to be a space */
    return (int)(sign * val);
}

/* This function mirrors the vulnerable 'c ' branch in runtest.c automataTest */
__attribute__((noinline))
static void automataTest_like(const char *line_in) {
    xmlAutomata am_obj; /* non-NULL sentinel */
    xmlAutomataPtr am = &am_obj;

    /* Small, fixed-size stack array to make the overflow easy to catch by ASan */
    xmlAutomataStatePtr states[4];
    for (int i = 0; i < 4; i++) states[i] = NULL;

    /* Duplicate the input because the original code uses char* arithmetic */
    char *expr = strdup(line_in);
    if (!expr) {
        fprintf(stderr, "OOM\n");
        return;
    }

    if ((am != NULL) && (expr[0] == 'c') && (expr[1] == ' ')) {
        char *ptr = &expr[2];
        int from, to;
        int min, max;

        from = scanNumber(&ptr);
        if (*ptr != ' ') {
            fprintf(stderr, "Bad line %s\n", expr);
            free(expr);
            return;
        }
        /* Vulnerable: 'from' is user-controlled and not bounds-checked. */
        if (states[from] == NULL)
            states[from] = xmlAutomataNewState(am); /* stack-buffer-overflow due to OOB index */
        ptr++;
        to = scanNumber(&ptr);
        if (*ptr != ' ') {
            fprintf(stderr, "Bad line %s\n", expr);
            free(expr);
            return;
        }
        if (states[to] == NULL)
            states[to] = xmlAutomataNewState(am);
        ptr++;
        min = scanNumber(&ptr);
        if (*ptr != ' ') {
            fprintf(stderr, "Bad line %s\n", expr);
            free(expr);
            return;
        }
        ptr++;
        max = scanNumber(&ptr);
        if (*ptr != ' ') {
            fprintf(stderr, "Bad line %s\n", expr);
            free(expr);
            return;
        }
        ptr++;
        xmlAutomataNewCountTrans(am, states[from], states[to], BAD_CAST ptr, min, max, NULL);
    }

    free(expr);
}

int main(void) {
    /* Craft a line matching the 'c ' branch format used by runtest.c:
     *   c <from> <to> <min> <max> <text> 
     * Choose from=5, while states has only 4 entries, to trigger OOB.
     * Ensure spaces are present exactly where runtest.c checks them.
     */
    const char *evil = "c 5 0 1 2 payload ";
    automataTest_like(evil);
    return 0;
}
