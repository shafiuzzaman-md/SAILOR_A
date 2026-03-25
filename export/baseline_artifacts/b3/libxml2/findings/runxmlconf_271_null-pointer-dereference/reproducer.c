// Standalone reproducer for NULL dereference in xmlconfTestNotWF
// It simulates xmlNewParserCtxt() failing and then dereferencing the NULL
// context in xmlCtxtSetErrorHandler(ctxt, ...).

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// Minimal type stubs to avoid linking against libxml2
typedef struct _xmlParserCtxt { int dummy; } xmlParserCtxt;
typedef xmlParserCtxt* xmlParserCtxtPtr;

typedef struct _xmlDoc { int dummy; } xmlDoc;
typedef xmlDoc* xmlDocPtr;

typedef void (*xmlGenericErrorFunc)(void *ctx, const char *msg, ...);

// Globals referenced by the test code
static int nb_errors = 0;

static void test_log(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

// Error handler type used in xmlCtxtSetErrorHandler
static void testErrorHandler(void *ctx, const char *msg, ...) {
    (void)ctx; (void)msg;
}

// Stub: Force allocation failure as described by the bug
static xmlParserCtxtPtr xmlNewParserCtxt(void) {
    return NULL; // simulate allocation failure
}

// Stub of the API that will dereference the ctxt unconditionally,
// mirroring the vulnerability when ctxt is NULL
static void xmlCtxtSetErrorHandler(xmlParserCtxtPtr ctxt,
                                   xmlGenericErrorFunc handler,
                                   void *ctx) {
    if (handler) handler(ctx, "setting error handler\n");
    // Intentional dereference to simulate libxml2 internals using ctxt.
    // This will crash when ctxt is NULL, demonstrating the bug.
    volatile int touch = ctxt->dummy; // NULL deref here
    (void)touch;
}

// Other stubs used by the vulnerable function
static xmlDocPtr xmlCtxtReadFile(xmlParserCtxtPtr ctxt, const char *filename,
                                 const char *encoding, int options) {
    (void)ctxt; (void)filename; (void)encoding; (void)options;
    return NULL;
}

static void xmlFreeDoc(xmlDocPtr doc) { (void)doc; }
static void xmlFreeParserCtxt(xmlParserCtxtPtr ctxt) { (void)ctxt; }

// Vulnerable function from runxmlconf.c (trimmed to the relevant lines)
static int xmlconfTestNotWF(const char *id, const char *filename, int options) {
    xmlParserCtxtPtr ctxt;
    xmlDocPtr doc;
    int ret = 1;

    ctxt = xmlNewParserCtxt();
    // Vulnerability: ctxt can be NULL but is used unconditionally
    xmlCtxtSetErrorHandler(ctxt, testErrorHandler, NULL);
    doc = xmlCtxtReadFile(ctxt, filename, NULL, options);
    if (doc != NULL) {
        test_log("test %s : %s failed to detect not well formedness\n",
                 id, filename);
        nb_errors++;
        xmlFreeDoc(doc);
        ret = 0;
    }
    xmlFreeParserCtxt(ctxt);
    return ret;
}

int main(void) {
    // Any filename/options are fine; crash occurs before they're used
    const char *id = "repro";
    const char *filename = "/dev/null";
    int options = 0;

    // This call will trigger the NULL dereference inside xmlCtxtSetErrorHandler
    // because xmlNewParserCtxt() returns NULL.
    int r = xmlconfTestNotWF(id, filename, options);

    // We should not reach here due to the crash
    printf("Returned %d, nb_errors=%d\n", r, nb_errors);
    return 0;
}
