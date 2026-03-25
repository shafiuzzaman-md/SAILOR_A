#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * Minimal stand-ins for the libxml2 memory and XPath APIs, sufficient to
 * reproduce the NULL dereference in initializeLibxml2 when
 * xmlXPathNewContext(NULL) returns NULL (simulating OOM).
 */

typedef void (*xmlFreeFunc)(void *ptr);
typedef void *(*xmlMallocFunc)(size_t size);
typedef void *(*xmlReallocFunc)(void *ptr, size_t size);
typedef char *(*xmlStrdupFunc)(const char *str);

static xmlFreeFunc    g_free_func;
static xmlMallocFunc  g_malloc_func;
static xmlReallocFunc g_realloc_func;
static xmlStrdupFunc  g_strdup_func;

int xmlMemSetup(xmlFreeFunc freeFunc, xmlMallocFunc mallocFunc,
                xmlReallocFunc reallocFunc, xmlStrdupFunc strdupFunc) {
    g_free_func = freeFunc;
    g_malloc_func = mallocFunc;
    g_realloc_func = reallocFunc;
    g_strdup_func = strdupFunc;
    return 0;
}

/* Default-like memory wrappers (names mirror libxml2 API). */
void xmlMemFree(void *ptr) {
    free(ptr);
}

/* Intentionally fail to simulate OOM in xmlXPathNewContext. */
void *xmlMemMalloc(size_t size) {
    (void)size;
    return NULL; /* Always fail: forces xmlXPathNewContext to return NULL */
}

void *xmlMemRealloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

char *xmlMemoryStrdup(const char *str) {
    return strdup(str);
}

/* Stub libxml2 initialization APIs. */
void xmlInitParser(void) {
    /* no-op for this reproducer */
}

/* Minimal XPath context definition with a cache field, as in libxml2. */
typedef struct _xmlXPathContext {
    void *cache; /* this is what the vulnerable code dereferences */
} xmlXPathContext;

typedef xmlXPathContext *xmlXPathContextPtr;

/* Public API stub that uses the configured memory hooks. */
xmlXPathContextPtr xmlXPathNewContext(void *node) {
    (void)node; /* unused */
    xmlXPathContextPtr ctx = NULL;
    if (g_malloc_func) {
        ctx = (xmlXPathContextPtr)g_malloc_func(sizeof(xmlXPathContext));
    } else {
        ctx = (xmlXPathContextPtr)malloc(sizeof(xmlXPathContext));
    }
    if (ctx == NULL) {
        return NULL; /* Simulated OOM path */
    }
    ctx->cache = NULL;
    return ctx;
}

/* Stub for completeness; never reached before the crash. */
void xmlXPathContextSetCache(xmlXPathContextPtr ctx, int active, int value, int max) {
    (void)ctx; (void)active; (void)value; (void)max;
}

/* ---------------- Vulnerable logic replica ---------------- */
static xmlXPathContextPtr ctxtXPath;

static void initializeLibxml2(void) {
    xmlMemSetup(xmlMemFree, xmlMemMalloc, xmlMemRealloc, xmlMemoryStrdup);
    xmlInitParser();
    /* Skipping catalog setup (#ifdef LIBXML_CATALOG_ENABLED) */

    /* This will return NULL because xmlMemMalloc always fails. */
    ctxtXPath = xmlXPathNewContext(NULL);

    /* Vulnerable NULL dereference: ctxtXPath is NULL, deref of ->cache crashes */
    if (ctxtXPath->cache != NULL)
        xmlXPathContextSetCache(ctxtXPath, 0, -1, 0);
}

int main(void) {
    /* Trigger the vulnerable code path. */
    initializeLibxml2();
    /* We should never reach here; if we do, print something. */
    printf("If you see this, the NULL dereference did not trigger.\n");
    return 0;
}
