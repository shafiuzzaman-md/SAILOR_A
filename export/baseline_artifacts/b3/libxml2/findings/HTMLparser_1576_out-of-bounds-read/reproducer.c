#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal re-declarations from libxml2 */
typedef unsigned char xmlChar;

static int xmlStrEqual(const xmlChar *a, const xmlChar *b) {
    if (a == NULL || b == NULL)
        return 0;
    while (*a && *b) {
        if (*a != *b)
            return 0;
        a++; b++;
    }
    return (*a == 0 && *b == 0);
}

/* Dummy table so the function compiles; it's not reached in this repro */
static const char *htmlScriptAttributes[] = {
    "onclick", "onload", "onerror"
};

__attribute__((noinline))
int htmlIsScriptAttribute(const xmlChar *name) {
    unsigned int i;

    if (name == NULL)
        return 0;
    /* Vulnerable check: may read name[1] without ensuring length >= 2 */
    if ((name[0] != 'o') || (name[1] != 'n'))
        return 0;
    for (i = 0; i < sizeof(htmlScriptAttributes)/sizeof(htmlScriptAttributes[0]); i++) {
        if (xmlStrEqual(name, (const xmlChar *) htmlScriptAttributes[i]))
            return 1;
    }
    return 0;
}

int main(void) {
    /* Allocate a 1-byte buffer so that reading name[1] goes OOB. */
    xmlChar *name = (xmlChar *)malloc(1);
    if (name == NULL)
        return 1;

    /* Set name[0] to 'o' so the left side of the || is false and
       the code evaluates name[1], which is out-of-bounds. */
    name[0] = 'o';  /* Note: not NUL-terminated on purpose */

    int res = htmlIsScriptAttribute(name);
    printf("Result: %d\n", res);

    free(name);
    return 0;
}
