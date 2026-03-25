#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stubs to satisfy the vulnerable code's calls */
typedef void xmlAutomata;
typedef void xmlAutomataState;

#define BAD_CAST (const unsigned char *)

static xmlAutomata *xmlNewAutomata(void) {
    return (xmlAutomata *)malloc(1);
}

static void xmlFreeAutomata(xmlAutomata *am) {
    free(am);
}

static xmlAutomataState *xmlAutomataGetInitState(xmlAutomata *am) {
    /* Return a non-NULL pointer */
    return (xmlAutomataState *)((char *)am + 1);
}

static xmlAutomataState *xmlAutomataNewState(xmlAutomata *am) {
    (void)am;
    return (xmlAutomataState *)malloc(1);
}

static void xmlAutomataNewTransition(xmlAutomata *am,
                                     xmlAutomataState *from,
                                     xmlAutomataState *to,
                                     const unsigned char *val,
                                     void *data) {
    (void)am; (void)from; (void)to; (void)val; (void)data;
}

static void xmlAutomataNewEpsilon(xmlAutomata *am,
                                  xmlAutomataState *from,
                                  xmlAutomataState *to) {
    (void)am; (void)from; (void)to;
}

/* Simple number scanner similar to what's used by runtest.c */
static int scanNumber(char **pptr) {
    char *cur = *pptr;
    int sign = 1;
    int ret = 0;

    /* Optional leading spaces */
    while (*cur == ' ') cur++;
    if (*cur == '+') { cur++; }
    else if (*cur == '-') { sign = -1; cur++; }

    while (*cur >= '0' && *cur <= '9') {
        ret = ret * 10 + (*cur - '0');
        cur++;
    }
    *pptr = cur;
    return sign * ret;
}

/* Reimplementation of the vulnerable automataTest logic focusing on the 't ' branch */
static int automataTest_like(void) {
    FILE *input = tmpfile();
    if (input == NULL) {
        perror("tmpfile");
        return -1;
    }

    /* Craft input: 't ' line with from = 1000 (out of bounds for states[1000]) */
    fputs("t 1000 0 x\n", input);
    fflush(input);
    rewind(input);

    xmlAutomata *am = NULL;
    xmlAutomataState *states[1000];
    char expr[4500];
    int ret = 0;

    /* Initialize states[] to NULLs similar to real code's initial state handling */
    memset(states, 0, sizeof(states));

    am = xmlNewAutomata();
    if (am == NULL) {
        fprintf(stderr, "Cannot create automata\n");
        fclose(input);
        return -1;
    }

    states[0] = xmlAutomataGetInitState(am);
    if (states[0] == NULL) {
        fprintf(stderr, "Cannot get start state\n");
        xmlFreeAutomata(am);
        fclose(input);
        return -1;
    }

    while (fgets(expr, (int)sizeof(expr), input) != NULL) {
        if (expr[0] == '#')
            continue;
        int len = (int)strlen(expr);
        len--;
        while ((len >= 0) &&
               (expr[len] == '\n' || expr[len] == '\t' ||
                expr[len] == '\r' || expr[len] == ' ')) len--;
        expr[len + 1] = '\0';
        if (len >= 0) {
            if ((am != NULL) && (expr[0] == 't') && (expr[1] == ' ')) {
                char *ptr = &expr[2];
                int from, to;

                from = scanNumber(&ptr);
                if (*ptr != ' ') {
                    fprintf(stderr, "Bad line %s\n", expr);
                    break;
                }
                /* Vulnerable access: from can be >= 1000, reading states[1000] */
                if (states[from] == NULL)
                    /* Vulnerable write: writing states[1000] */
                    states[from] = xmlAutomataNewState(am);
                ptr++;
                to = scanNumber(&ptr);
                if (*ptr != ' ') {
                    fprintf(stderr, "Bad line %s\n", expr);
                    break;
                }
                if (states[to] == NULL)
                    states[to] = xmlAutomataNewState(am);
                ptr++;
                xmlAutomataNewTransition(am, states[from], states[to], BAD_CAST ptr, NULL);
            }
        }
    }

    fclose(input);
    xmlFreeAutomata(am);
    return ret;
}

int main(void) {
    /* This should trigger AddressSanitizer with a stack-buffer-overflow */
    return automataTest_like();
}
