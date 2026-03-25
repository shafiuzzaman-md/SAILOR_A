#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for libxml2 types and macros */
typedef unsigned char xmlChar;
typedef struct _xmlParserCtxt { int dummy; } *xmlParserCtxtPtr;
typedef struct _xmlDoc { int dummy; } *xmlDocPtr;
typedef struct _xmlNode { int dummy; } *xmlNodePtr;

#ifndef BAD_CAST
#define BAD_CAST (xmlChar *)
#endif

/* Stub implementations mimicking the libxml2 API used by example/icu.c */
static xmlParserCtxtPtr xmlNewParserCtxt(void) {
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr)malloc(sizeof(*ctxt));
    return ctxt;
}

static void xmlFreeParserCtxt(xmlParserCtxtPtr ctxt) {
    free(ctxt);
}

/* Signature relaxed to accept generic pointers to avoid dependency on real prototypes */
static void xmlCtxtSetCharEncConvImpl(xmlParserCtxtPtr ctxt, void *convImpl, void *userdata) {
    (void)ctxt; (void)convImpl; (void)userdata;
}

/* Force a read failure to simulate unsupported encoding / ICU setup failure */
static xmlDocPtr xmlCtxtReadDoc(xmlParserCtxtPtr ctxt, const xmlChar *cur,
                                const char *URL, const char *encoding, int options) {
    (void)ctxt; (void)cur; (void)URL; (void)encoding; (void)options;
    /* Simulate failure: return NULL document */
    return NULL;
}

/* Mimic libxml2: passing NULL returns NULL */
static xmlChar *xmlNodeGetContent(xmlNodePtr node) {
    (void)node;
    return NULL;
}

static int xmlStrEqual(const xmlChar *str1, const xmlChar *str2) {
    if (str1 == NULL || str2 == NULL) return 0;
    return strcmp((const char *)str1, (const char *)str2) == 0;
}

static void xmlFree(void *ptr) {
    free(ptr);
}

static void xmlFreeDoc(xmlDocPtr doc) {
    free(doc);
}

/* Dummy symbol to match example usage; not actually invoked */
static void *icuConvImpl = (void*)0x1;

int main(void) {
    xmlParserCtxtPtr ctxt;
    xmlDocPtr doc;
    const char *xml;
    xmlChar *content;
    int ret = 0;

    /* Same payload as example/icu.c */
    xml = "<doc>\xDE</doc>";

    ctxt = xmlNewParserCtxt();
    xmlCtxtSetCharEncConvImpl(ctxt, icuConvImpl, NULL);
    /* This stub returns NULL to simulate a parse failure */
    doc = xmlCtxtReadDoc(ctxt, BAD_CAST xml, NULL, "IBM-1051", 0);
    xmlFreeParserCtxt(ctxt);

    /* Passing NULL here returns NULL, as in libxml2 */
    content = xmlNodeGetContent((xmlNodePtr) doc);

    /* BUG: printf with %s and a NULL pointer triggers a NULL dereference on many libc impls */
    printf("content: %s\n", content);

    /* The rest mirrors the example; likely not reached if the line above crashes */
    if (!xmlStrEqual(content, BAD_CAST "\xC3\x9F")) {
        fprintf(stderr, "conversion failed\n");
        ret = 1;
    }

    xmlFree(content);
    xmlFreeDoc(doc);

    return ret;
}
