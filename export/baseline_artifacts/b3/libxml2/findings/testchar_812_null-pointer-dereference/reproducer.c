#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal public type re-declarations to match libxml2 API signatures */
typedef struct _xmlParserCtxt {
    int dummy;
} xmlParserCtxt;

typedef xmlParserCtxt *xmlParserCtxtPtr;

typedef struct _xmlSAXHandler xmlSAXHandler;
typedef xmlSAXHandler *xmlSAXHandlerPtr;

/* Stub of xmlCreatePushParserCtxt that simulates allocation failure by
 * returning NULL (as would happen on OOM inside libxml2). */
xmlParserCtxtPtr xmlCreatePushParserCtxt(xmlSAXHandlerPtr sax, void *user_data,
                                        const char *chunk, int size,
                                        const char *filename) {
    (void)sax; (void)user_data; (void)chunk; (void)size; (void)filename;
    return NULL; /* simulate allocation failure */
}

/* Stub of xmlParseChunk that unconditionally dereferences the parser context
 * pointer, mirroring the vulnerable behavior when the caller does not check
 * for NULL. */
int xmlParseChunk(xmlParserCtxtPtr ctxt, const char *chunk, int size, int terminate) {
    (void)chunk; (void)size; (void)terminate;
    /* Intentional NULL dereference if ctxt is NULL to reproduce the bug path */
    volatile int *p = (int *)ctxt;
    /* this read will crash if ctxt is NULL */
    int v = *p;
    /* prevent unused warning */
    return v;
}

int main(void) {
    /* This mirrors the vulnerable sequence in testUTF8Chunks: */
    xmlParserCtxtPtr ctxt = xmlCreatePushParserCtxt(NULL, NULL, NULL, 0, NULL);

    /* Missing NULL check on ctxt, immediately calling into xmlParseChunk
     * which will dereference ctxt and crash. */
    xmlParseChunk(ctxt, "<d>", 3, 0);

    /* Unreachable */
    return 0;
}
