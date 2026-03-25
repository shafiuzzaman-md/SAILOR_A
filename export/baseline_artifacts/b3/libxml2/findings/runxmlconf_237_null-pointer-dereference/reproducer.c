#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/* Minimal stand-ins for libxml2 types/constants used by runxmlconf.c */
typedef struct _xmlParserCtxt {
    int dummy;
    int valid;
} xmlParserCtxt, *xmlParserCtxtPtr;

typedef struct _xmlDoc {
    int properties;
} xmlDoc, *xmlDocPtr;

typedef struct _xmlError {
    int domain;
    int code;
} xmlError;

typedef void (*xmlErrorFunc)(void *ctx, const char *msg, ...);

#define XML_DOC_DTDVALID (1<<0)
#define XML_ERR_OK 0
#define XML_FROM_NAMESPACE 100

/* Global test infra stubs (as referenced by the original file) */
static int nb_errors = 0;
static void test_log(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
static void testErrorHandler(void *ctx, const char *msg, ...) {
    (void)ctx; (void)msg; /* no-op */
}

/* Stubbed libxml2-like APIs */
static xmlParserCtxtPtr xmlNewParserCtxt(void) {
    /* Simulate allocation failure: return NULL to reproduce the bug path */
    return NULL;
}

static void xmlCtxtSetErrorHandler(xmlParserCtxtPtr ctxt,
                                   xmlErrorFunc handler,
                                   void *ctx) {
    (void)handler; (void)ctx;
    /* This intentionally dereferences ctxt, reproducing the NPD when ctxt==NULL */
    ctxt->dummy = 1; /* Null-pointer dereference here */
}

static xmlDocPtr xmlCtxtReadFile(xmlParserCtxtPtr ctxt, const char *filename,
                                 const char *encoding, int options) {
    (void)ctxt; (void)filename; (void)encoding; (void)options;
    return NULL;
}

static const xmlError* xmlGetLastError(void) {
    static xmlError e = { XML_FROM_NAMESPACE, XML_ERR_OK };
    return &e;
}

static void xmlFreeDoc(xmlDocPtr doc) { (void)doc; }
static void xmlFreeParserCtxt(xmlParserCtxtPtr ctxt) { (void)ctxt; }

/* Vulnerable function from runxmlconf.c (trimmed to relevant parts) */
static int xmlconfTestNotNSWF(const char *id, const char *filename, int options) {
    xmlParserCtxtPtr ctxt;
    xmlDocPtr doc;
    int ret = 1;

    ctxt = xmlNewParserCtxt();
    /* Missing NULL check here is the bug: next call dereferences ctxt unconditionally */
    xmlCtxtSetErrorHandler(ctxt, testErrorHandler, NULL);
    /* In case of Namespace errors, libxml2 will still parse the document
     * but log a Namespace error. */
    doc = xmlCtxtReadFile(ctxt, filename, NULL, options);
    if (doc == NULL) {
        test_log("test %s : %s failed to parse the XML\n", id, filename);
        nb_errors++;
        ret = 0;
    } else {
        const xmlError *error = xmlGetLastError();
        if ((error->code == XML_ERR_OK) || (error->domain != XML_FROM_NAMESPACE)) {
            test_log("test %s : %s failed to detect namespace error\n", id, filename);
            nb_errors++;
            ret = 0;
        }
        xmlFreeDoc(doc);
    }
    xmlFreeParserCtxt(ctxt);
    return ret;
}

int main(void) {
    /* Any inputs are fine; the crash happens before parsing due to NULL ctxt */
    fprintf(stderr, "Triggering xmlconfTestNotNSWF null-deref via NULL parser context...\n");
    /* This call will crash inside xmlCtxtSetErrorHandler because ctxt==NULL */
    (void)xmlconfTestNotNSWF("id", "dummy.xml", 0);
    return 0;
}
