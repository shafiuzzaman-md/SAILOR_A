#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* Minimal type re-declarations to mirror libxml2 test code context */
typedef unsigned char xmlChar;
typedef struct _xmlDoc xmlDoc;
typedef struct _xmlParserCtxt xmlParserCtxt;
typedef xmlParserCtxt* xmlParserCtxtPtr;

struct _xmlDoc {
    int dummy;
};

struct _xmlParserCtxt {
    void *myDoc;
};

/* Stub implementations of libxml2 APIs used by testUTF8Chunks */
xmlParserCtxtPtr xmlCreatePushParserCtxt(void *sax, void *user_data,
                                         const char *chunk, int size,
                                         const char *filename) {
    (void)sax; (void)user_data; (void)chunk; (void)size; (void)filename;
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr)malloc(sizeof(xmlParserCtxt));
    if (ctxt) ctxt->myDoc = malloc(sizeof(xmlDoc));
    return ctxt;
}

int xmlParseChunk(xmlParserCtxtPtr ctxt, const char *chunk, int size, int terminate) {
    (void)ctxt; (void)chunk; (void)size; (void)terminate;
    return 0;
}

void xmlDocDumpMemory(void *doc, xmlChar **out, int *outSize) {
    (void)doc;
    const char *s = "<?xml version=\"1.0\"?>\n<d></d>\n";
    *outSize = (int)strlen(s);
    *out = (xmlChar*)strdup(s);
}

void xmlFree(void *ptr) {
    free(ptr);
}

void xmlFreeDoc(void *doc) {
    free(doc);
}

void xmlFreeParserCtxt(xmlParserCtxtPtr ctxt) {
    if (!ctxt) return;
    free(ctxt->myDoc);
    free(ctxt);
}

/* Critical piece: xmlMalloc returns NULL to simulate allocation failure */
void *xmlMalloc(size_t size) {
    (void)size;
    return NULL; /* Force allocation failure */
}

/* Reproduction of the vulnerable portion of testUTF8Chunks (testchar.c:840) */
static int trigger_null_deref_in_testUTF8Chunks(void) {
    xmlParserCtxtPtr ctxt;
    char *buf;
    int i;

    /* Setup as in the original test before the vulnerable allocation */
    ctxt = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL);
    xmlParseChunk(ctxt, "<d>", 3, 0);

    /* This allocation fails (xmlMalloc returns NULL), but the code doesn't check */
    buf = (char *)xmlMalloc(1000 * 2 + 1);

    /* Vulnerable loop: dereferences buf even if NULL */
    for (i = 0; i < 2000; i += 2) {
        /* Corresponds to testchar.c:840 - memcpy(buf + i, "\xCE\xB1", 2); */
        memcpy(buf + i, "\xCE\xB1", 2); /* NULL destination -> crash */
    }

    /* Not reached, but keep for structural similarity */
    (void)ctxt;
    return 0;
}

int main(void) {
    /* ASan should report a NULL-dereference here */
    return trigger_null_deref_in_testUTF8Chunks();
}
