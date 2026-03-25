#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal libxml2-like typedefs and macros */
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

typedef struct _xmlNode {
    struct _xmlNode *children;
    xmlChar *content;
} xmlNode, *xmlNodePtr;

typedef struct _xmlDoc {
    xmlNodePtr children;
} xmlDoc, *xmlDocPtr;

typedef struct _xmlParserCtxt {
    xmlDocPtr myDoc;
    int dummy;
} xmlParserCtxt, *xmlParserCtxtPtr;

typedef struct _xmlCharEncodingHandler {
    int dummy;
} xmlCharEncodingHandler, *xmlCharEncodingHandlerPtr;

/* Dummy encoding enum */
enum {
    XML_CHAR_ENCODING_UTF16LE = 1
};

/* Stubbed public API functions to simulate the vulnerable behavior */
static xmlCharEncodingHandlerPtr xmlGetCharEncodingHandler(int enc) {
    (void)enc;
    /* Return a non-NULL dummy handler */
    static xmlCharEncodingHandler handler;
    return &handler;
}

/* Simplified version of convert() which just duplicates the input buffer. */
static char *convert(xmlCharEncodingHandlerPtr handler, const char *utf8, int size, int *outSize) {
    (void)handler;
    if (size < 0) return NULL;
    char *ret = (char *)malloc((size_t)size);
    if (!ret) return NULL;
    memcpy(ret, utf8, (size_t)size);
    if (outSize)
        *outSize = size;
    return ret;
}

/* Simulate allocation failure in the push parser context creator. */
static xmlParserCtxtPtr xmlCreatePushParserCtxt(void *sax, void *user_data, const char *chunk, int size, const char *filename) {
    (void)sax; (void)user_data; (void)chunk; (void)size; (void)filename;
    /* Bug trigger: return NULL to mimic allocation failure */
    return NULL;
}

/* Vulnerable function: will dereference ctxt without NULL check. */
static int xmlSwitchEncoding(xmlParserCtxtPtr ctxt, int enc) {
    (void)enc;
    /* Dereference ctxt directly to simulate libxml2's behavior of using ctxt */
    /* This will crash if ctxt == NULL, reproducing the NULL dereference. */
    return ctxt->dummy; /* Intentional NULL dereference when ctxt is NULL */
}

/* No-op stubs for the remaining API used later in the test. */
static void xmlParseChunk(xmlParserCtxtPtr ctxt, const char *chunk, int size, int terminate) {
    (void)ctxt; (void)chunk; (void)size; (void)terminate;
}

static int xmlStrcmp(const xmlChar *a, const xmlChar *b) {
    if (a == NULL && b == NULL) return 0;
    if (a == NULL) return -1;
    if (b == NULL) return 1;
    return strcmp((const char *)a, (const char *)b);
}

static void xmlFreeDoc(xmlDocPtr doc) {
    if (!doc) return;
    /* Minimal free: only free top-level node content if allocated by us. */
    if (doc->children) {
        free(doc->children->content);
        free(doc->children);
    }
    free(doc);
}

static void xmlFreeParserCtxt(xmlParserCtxtPtr ctxt) {
    free(ctxt);
}

static void xmlFree(void *p) {
    free(p);
}

/* Reimplementation of the test function containing the vulnerable call chain. */
static int testUserEncodingPush(void) {
    xmlCharEncodingHandlerPtr handler;
    xmlParserCtxtPtr ctxt;
    xmlDocPtr doc;
    char buf[] =
        "\xEF\xBB\xBF"
        "<?xml version='1.0' encoding='ISO-8859-1'?>\n"
        "<d>text</d>\n";
    char *utf16;
    int utf16Size;
    int ret = 1;

    handler = xmlGetCharEncodingHandler(XML_CHAR_ENCODING_UTF16LE);
    utf16 = convert(handler, buf, (int)sizeof(buf) - 1, &utf16Size);

    /* Intentionally returns NULL in our stub to simulate allocation failure */
    ctxt = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL);

    /* Vulnerable call: ctxt is NULL, xmlSwitchEncoding will dereference it */
    xmlSwitchEncoding(ctxt, XML_CHAR_ENCODING_UTF16LE);

    /* Unreached in this reproducer due to crash above */
    xmlParseChunk(ctxt, utf16, utf16Size, 0);
    xmlParseChunk(ctxt, NULL, 0, 1);
    doc = ctxt ? ctxt->myDoc : NULL;

    if ((doc != NULL) &&
        (doc->children != NULL) &&
        (doc->children->children != NULL) &&
        (xmlStrcmp(doc->children->children->content, BAD_CAST "text") == 0))
        ret = 0;

    xmlFreeDoc(doc);
    xmlFreeParserCtxt(ctxt);
    xmlFree(utf16);

    return ret;
}

int main(void) {
    /* Execute the test which will trigger the NULL pointer dereference */
    (void)testUserEncodingPush();
    return 0;
}
