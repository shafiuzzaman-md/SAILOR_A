#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>

/* Minimal libxml2-like typedefs and macros */
typedef unsigned char xmlChar;
typedef void* xmlNodePtr;
#define BAD_CAST (xmlChar *)

/* Simple memory accounting to mimic xmlMemUsed() */
static size_t g_memused = 0;

static void* xmlMalloc(size_t size) {
    /* store size before the returned pointer to track frees */
    size_t total = sizeof(size_t) + size;
    size_t *base = (size_t*)malloc(total);
    if (!base) return NULL;
    *base = size;
    g_memused += size;
    return (void*)(base + 1);
}

static void xmlFree(void *ptr) {
    if (!ptr) return;
    size_t *base = ((size_t*)ptr) - 1;
    g_memused -= *base;
    free(base);
}

static int xmlMemUsed(void) {
    return (int)g_memused;
}

static xmlChar* xmlStrdup(const xmlChar *s) {
    if (!s) return NULL;
    size_t len = strlen((const char*)s);
    xmlChar *p = (xmlChar*)xmlMalloc(len + 1);
    if (!p) return NULL;
    memcpy(p, s, len + 1);
    return p;
}

/* Stubs matching the original environment */
static long xmlGetLineNo(xmlNodePtr cur) {
    (void)cur;
    return 1234; /* arbitrary line number */
}

static void xmlResetLastError(void) {
    /* no-op */
}

/* A tiny strlen that will be instrumented by ASan and triggers on UAF */
static size_t my_strlen_asan(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') n++;  /* ASan will check each load here */
    return n;
}

/* Minimal test_log that processes just "%ld", "%s", and "%d" used below */
static void test_log(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    const char *p = fmt;
    while (*p) {
        if (*p == '%' && *(p+1)) {
            ++p;
            if (*p == 'l' && *(p+1) == 'd') {
                ++p; /* skip 'd' */
                long v = va_arg(ap, long);
                fprintf(stderr, "%ld", v);
            } else if (*p == 'd') {
                int v = va_arg(ap, int);
                fprintf(stderr, "%d", v);
            } else if (*p == 's') {
                const char *s = va_arg(ap, const char*);
                /* This read from freed memory triggers ASan */
                size_t len = my_strlen_asan(s);
                fwrite(s, 1, len, stderr);
            } else {
                /* Fallback: print specifier literally if unexpected */
                fputc('%', stderr);
                fputc(*p, stderr);
            }
        } else {
            fputc(*p, stderr);
        }
        ++p;
    }
    va_end(ap);
}

/* Reproducer for the bug in runsuite.c:xstcTestGroup cleanup block */
static int xstcTestGroup_reproducer(void) {
    /* In the real code, 'mem' is captured earlier and compared at the end. */
    int mem = xmlMemUsed();

    /* Allocate a path string, as in the real code. */
    xmlChar *path = xmlStrdup(BAD_CAST "dummy.xsd");
    if (!path) {
        fprintf(stderr, "allocation failed\n");
        return -1;
    }

    /* Simulate scope variables used by the log. */
    xmlNodePtr cur = (xmlNodePtr)0x1; /* dummy node pointer */

    /* --- cleanup block begins --- */
    /* href, validity, schemas would be freed here too in the real code. */

    /* Free 'path' first, exactly like line 1138 in the original code. */
    if (path != NULL) xmlFree(path);

    /* Create a deliberate leak after taking the initial 'mem' snapshot,
       so that mem != xmlMemUsed() and the logging path is taken. */
    static void *leak_guard; /* keep it reachable */
    leak_guard = xmlMalloc(16); /* leaked on purpose */
    (void)leak_guard;

    xmlResetLastError();

    /* This mirrors line 1142-1144 in the original code. It wrongly uses
       the freed 'path' in the %s argument, triggering UAF. */
    if (mem != xmlMemUsed()) {
        test_log("Processing test line %ld %s leaked %d\n",
                 xmlGetLineNo(cur), (const char*)path, xmlMemUsed() - mem);
    }
    /* --- cleanup block ends --- */

    return 0;
}

int main(void) {
    return xstcTestGroup_reproducer();
}
