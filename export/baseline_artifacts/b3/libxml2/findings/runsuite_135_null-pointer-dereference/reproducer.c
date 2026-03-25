#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* Minimal type and API stubs to mimic the libxml2 interfaces used */
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

typedef void (*xmlFreeFunc)(void *);
typedef void *(*xmlMallocFunc)(size_t);
typedef void *(*xmlReallocFunc)(void *, size_t);
typedef char *(*xmlStrdupFunc)(const char *);

typedef struct _xmlXPathContext {
    void *doc;
    void *node;
    void *cache;
} xmlXPathContext;
typedef xmlXPathContext* xmlXPathContextPtr;

/* libxml2 memory API stubs */
int xmlMemSetup(xmlFreeFunc freeFunc, xmlMallocFunc mallocFunc,
                xmlReallocFunc reallocFunc, xmlStrdupFunc strdupFunc) {
    (void)freeFunc; (void)mallocFunc; (void)reallocFunc; (void)strdupFunc;
    return 0;
}
void xmlMemFree(void *p) { free(p); }
void *xmlMemMalloc(size_t s) { return malloc(s); }
void *xmlMemRealloc(void *p, size_t s) { return realloc(p, s); }
char *xmlMemoryStrdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *d = (char *)malloc(n);
    if (d) memcpy(d, s, n);
    return d;
}

/* Other libxml2 API stubs used by initializeLibxml2 */
void xmlInitParser(void) {}

/* Intentionally simulate allocation failure -> return NULL */
xmlXPathContextPtr xmlXPathNewContext(void *node) {
    (void)node;
    return NULL;
}

void xmlXPathContextSetCache(xmlXPathContextPtr ctxt, int active, int value, int options) {
    (void)ctxt; (void)active; (void)value; (void)options;
}

int xmlXPathRegisterNs(xmlXPathContextPtr ctxt, const xmlChar *prefix, const xmlChar *href) {
    (void)ctxt; (void)prefix; (void)href;
    return 0;
}

typedef void (*xmlGenericErrorFunc)(void *ctx, const char *msg, ...);
void xmlSetGenericErrorFunc(void *ctx, xmlGenericErrorFunc handler) {
    (void)ctx; (void)handler;
}

static void testErrorHandler(void *ctx, const char *msg, ...) {
    (void)ctx; (void)msg;
    va_list ap;
    va_start(ap, msg);
    va_end(ap);
}

/* Global context like in runsuite.c */
static xmlXPathContextPtr ctxtXPath = NULL;

/* Reimplementation of the vulnerable function from runsuite.c */
static void initializeLibxml2(void) {
    xmlMemSetup(xmlMemFree, xmlMemMalloc, xmlMemRealloc, xmlMemoryStrdup);
    xmlInitParser();
    ctxtXPath = xmlXPathNewContext(NULL);
    /* Vulnerable dereference: ctxtXPath may be NULL if allocation failed */
    if (ctxtXPath->cache != NULL)
        xmlXPathContextSetCache(ctxtXPath, 0, -1, 0);
    /* Subsequent uses would also dereference NULL, but we already crash above */
    xmlXPathRegisterNs(ctxtXPath, BAD_CAST "ts", BAD_CAST "TestSuite");
    xmlXPathRegisterNs(ctxtXPath, BAD_CAST "xlink", BAD_CAST "http://www.w3.org/1999/xlink");
    xmlSetGenericErrorFunc(NULL, testErrorHandler);
}

int main(void) {
    /* Triggers the NULL pointer dereference */
    initializeLibxml2();
    return 0;
}
