#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Minimal type re-declarations to mimic libxml2 interfaces used by runxmlconf.c */
typedef struct _xmlParserCtxt {
    int dummy;
} xmlParserCtxt;

typedef xmlParserCtxt* xmlParserCtxtPtr;

typedef struct _xmlDoc {
    int dummy;
} xmlDoc;

typedef xmlDoc* xmlDocPtr;

typedef struct _xmlError {
    int domain;
    int code;
} xmlError;

/* Constants from libxml2 (values here are placeholders, not used due to NULL deref) */
#define XML_ERR_OK 0
#define XML_FROM_NAMESPACE 200

/* Stub implementations of the libxml2 API used by the vulnerable code */
xmlParserCtxtPtr xmlNewParserCtxt(void) {
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr)malloc(sizeof(xmlParserCtxt));
    return ctxt;
}

typedef void (*xmlErrorFunc)(void *ctx, const char *msg, ...);

void xmlCtxtSetErrorHandler(xmlParserCtxtPtr ctxt, xmlErrorFunc handler, void *ctx) {
    (void)ctxt; (void)handler; (void)ctx; /* no-op stub */
}

xmlDocPtr xmlCtxtReadFile(xmlParserCtxtPtr ctxt, const char *filename, const char *encoding, int options) {
    (void)ctxt; (void)filename; (void)encoding; (void)options;
    /* Return a non-NULL document to reach the vulnerable branch */
    return (xmlDocPtr)malloc(sizeof(xmlDoc));
}

const xmlError* xmlGetLastError(void) {
    /* Critical behavior: return NULL to simulate \"no error set\" */
    return NULL;
}

void xmlFreeDoc(xmlDocPtr doc) {
    free(doc);
}

void xmlFreeParserCtxt(xmlParserCtxtPtr ctxt) {
    free(ctxt);
}

/* Dummy error handler used in the test harness */
static void testErrorHandler(void *ctx, const char *msg, ...) {
    (void)ctx; (void)msg; /* no-op */
}

/* Vulnerable function reproduced from runxmlconf.c (trimmed to the faulty logic) */
static int xmlconfTestNotNSWF(const char *id, const char *filename, int options) {
    (void)id; /* unused in this minimal reproducer */
    xmlParserCtxtPtr ctxt;
    xmlDocPtr doc;
    int ret = 1;

    ctxt = xmlNewParserCtxt();
    xmlCtxtSetErrorHandler(ctxt, testErrorHandler, NULL);

    /* In case of Namespace errors, libxml2 will still parse the document
     * but log a Namespace error.
     */
    doc = xmlCtxtReadFile(ctxt, filename, NULL, options);
    if (doc == NULL) {
        ret = 0;
    } else {
        const xmlError *error = xmlGetLastError();
        /* BUG: error may be NULL; dereferencing causes a crash */
        if ((error->code == XML_ERR_OK) || (error->domain != XML_FROM_NAMESPACE)) {
            /* no-op, the crash happens before this can run */
            ret = 0;
        }
        xmlFreeDoc(doc);
    }
    xmlFreeParserCtxt(ctxt);
    return ret;
}

int main(void) {
    /* Trigger the vulnerable code path: ensure parsing \"succeeds\" and no error is set */
    (void)xmlconfTestNotNSWF("id-1", "dummy.xml", 0);
    /* If the bug is present, program will crash before reaching here */
    puts("If you see this, the NULL deref did not trigger.");
    return 0;
}
