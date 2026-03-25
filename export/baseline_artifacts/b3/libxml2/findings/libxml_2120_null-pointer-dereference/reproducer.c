#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/* Minimal type definitions mirroring the python/libxml.c usage */
typedef unsigned char xmlChar;

typedef struct {
    void *ctx;
    xmlChar *name;
    xmlChar *ns_uri;
    void *function;
} libxml_xpathCallback;

/* This mirrors the odd "pointer-to-array" pattern used in libxml2 */
typedef libxml_xpathCallback libxml_xpathCallbackArray[];

static libxml_xpathCallbackArray *libxml_xpathCallbacks = NULL;
static int libxml_xpathCallbacksInitialized = 0;
static int libxml_xpathCallbacksAllocd = 2; /* any positive value triggers the loop */

/* Stub xmlMalloc that forces allocation failure to trigger the bug */
void *xmlMalloc(size_t size) {
    (void)size;
    return NULL; /* Force failure */
}

/* Vulnerable function as in python/libxml.c */
static void libxml_xpathCallbacksInitialize(void)
{
    int i;

    if (libxml_xpathCallbacksInitialized != 0)
        return;

    libxml_xpathCallbacks = (libxml_xpathCallbackArray*)xmlMalloc(
            libxml_xpathCallbacksAllocd * sizeof(libxml_xpathCallback));

    /* BUG: libxml_xpathCallbacks is not checked for NULL before dereference */
    for (i = 0; i < libxml_xpathCallbacksAllocd; i++) {
        (*libxml_xpathCallbacks)[i].ctx = NULL;
        (*libxml_xpathCallbacks)[i].name = NULL;
        (*libxml_xpathCallbacks)[i].ns_uri = NULL;
        (*libxml_xpathCallbacks)[i].function = NULL;
    }
    libxml_xpathCallbacksInitialized = 1;
}

int main(void) {
    fprintf(stderr, "About to call libxml_xpathCallbacksInitialize (xmlMalloc forced to return NULL) ...\n");
    /* This call will NULL-deref inside the loop above */
    libxml_xpathCallbacksInitialize();

    /* Should never reach here */
    fprintf(stderr, "If you see this, the bug did not trigger.\n");
    return 0;
}
