// Standalone reproducer for null-pointer-dereference in runtest.c:pushParseTest at line 1907
// It simulates xmlCreatePushParserCtxt failure (returns NULL), and then calls
// xmlCtxtSetErrorHandler(ctxt, ...) without a NULL check, triggering a crash.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Minimal type stubs to mirror libxml2 API used by runtest.c

typedef struct _xmlDoc {
    int dummy;
} xmlDoc, *xmlDocPtr;

typedef struct _xmlParserCtxt {
    int wellFormed;
    xmlDocPtr myDoc;
    int dummy; // used to force dereference in the stub
} xmlParserCtxt, *xmlParserCtxtPtr;

// Constants used in the test code
#define XML_PARSE_HTML 1
#define XML_CHAR_ENCODING_NONE 0
#define ATTRIBUTE_UNUSED

// Global used by the test harness
static int nb_tests = 0;

// Prototypes for the functions used in pushParseTest
static int loadMem(const char *filename, const char **base, int *size);
static void testStructuredErrorHandler(void *userData, void *ctx, const char *msg, ...);

// Stubs emulating a subset of libxml2 API used by pushParseTest
static xmlParserCtxtPtr xmlCreatePushParserCtxt(void *sax, void *user_data,
                                                const char *chunk, int size,
                                                const char *filename) {
    (void)sax; (void)user_data; (void)chunk; (void)size; (void)filename;
    // Simulate allocation/creation failure as per vulnerability description
    return NULL;
}

static void xmlCtxtSetErrorHandler(xmlParserCtxtPtr ctxt,
                                   void (*handler)(void *userData, void *ctx, const char *msg, ...),
                                   void *userData) {
    (void)handler; (void)userData;
    // This mirrors the vulnerable dereference that occurs when ctxt is NULL.
    // In the real libxml2, the function would access fields in ctxt without a NULL check.
    // ASan will flag this as a NULL pointer dereference.
    ctxt->dummy = 1; // crash here when ctxt == NULL
}

static int xmlCtxtUseOptions(xmlParserCtxtPtr ctxt, int options) {
    (void)ctxt; (void)options; return 0;
}

static int xmlParseChunk(xmlParserCtxtPtr ctxt, const char *chunk, int size, int terminate) {
    (void)ctxt; (void)chunk; (void)size; (void)terminate; return 0;
}

static void xmlFreeParserCtxt(xmlParserCtxtPtr ctxt) { (void)ctxt; }
static void xmlFree(void *mem) { free(mem); }
static void xmlFreeDoc(xmlDocPtr doc) { (void)doc; }

// Minimal loader used by pushParseTest to obtain an in-memory buffer
static int loadMem(const char *filename, const char **base, int *size) {
    (void)filename;
    const char *sample = "<root/>";
    char *buf = (char *)malloc(strlen(sample) + 1);
    if (!buf)
        return -1;
    strcpy(buf, sample);
    *base = buf;
    *size = (int)strlen(sample);
    return 0;
}

static void testStructuredErrorHandler(void *userData, void *ctx, const char *msg, ...) {
    (void)userData; (void)ctx; (void)msg;
}

// Reimplementation of the vulnerable function from runtest.c
static int pushParseTest(const char *filename, const char *result,
                         const char *err ATTRIBUTE_UNUSED, int options) {
    xmlParserCtxtPtr ctxt;
    xmlDocPtr doc;
    const char *base;
    int size, res;
    int cur = 0;
    int chunkSize = 4;

    nb_tests++;
    if (loadMem(filename, &base, &size) != 0) {
        fprintf(stderr, "Failed to load %s\n", filename);
        return -1;
    }

    if (chunkSize > size)
        chunkSize = size;

    // Only the XML branch is used here; LIBXML_HTML_ENABLED is not defined in this reproducer
    ctxt = xmlCreatePushParserCtxt(NULL, NULL, base + cur, chunkSize, filename);

    // Vulnerable dereference: ctxt may be NULL
    xmlCtxtSetErrorHandler(ctxt, testStructuredErrorHandler, NULL);

    // The following lines are not reached due to the crash above, but are kept for fidelity
    xmlCtxtUseOptions(ctxt, options);
    cur += chunkSize;
    chunkSize = 1024;
    do {
        if (cur + chunkSize >= size) {
            xmlParseChunk(ctxt, base + cur, size - cur, 1);
            break;
        } else {
            xmlParseChunk(ctxt, base + cur, chunkSize, 0);
            cur += chunkSize;
        }
    } while (cur < size);
    doc = ctxt->myDoc;
    res = ctxt->wellFormed;
    xmlFreeParserCtxt(ctxt);
    xmlFree((char *)base);
    if (!res) {
        xmlFreeDoc(doc);
        fprintf(stderr, "Failed to parse %s\n", filename);
        return -1;
    }
    (void)result; // unused in this minimal reproducer
    return 0;
}

int main(void) {
    // Any filename works; loadMem ignores it and returns a small in-memory buffer
    // options = 0 ensures we take the XML (non-HTML) branch as in the provided snippet
    (void)pushParseTest("dummy.xml", "dummy.out", NULL, 0);
    return 0;
}
