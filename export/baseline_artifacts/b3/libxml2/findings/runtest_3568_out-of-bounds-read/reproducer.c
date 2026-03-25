#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Stub types to mirror the original code's expectations */
typedef void* xmlSchemaParserCtxtPtr;
typedef void* xmlSchemaPtr;

/* Global variables referenced by the original code */
int testErrorsSize = 0;
char *testErrors[1024];
int nb_tests = 0;

/* Stubs for functions referenced in schemasTest */
const char *baseFilename(const char *filename) {
    /* In the real code, this returns the basename component.
       For this reproducer, return the input directly. */
    return filename;
}

xmlSchemaParserCtxtPtr xmlSchemaNewParserCtxt(const char *filename) {
    (void)filename;
    return (void*)0x1;
}

void testStructuredErrorHandler(void *userData, void *error) {
    (void)userData; (void)error;
}

void xmlSchemaSetParserStructuredErrors(xmlSchemaParserCtxtPtr ctxt, void (*handler)(void*, void*), void *userData) {
    (void)ctxt; (void)handler; (void)userData;
}

xmlSchemaPtr xmlSchemaParse(xmlSchemaParserCtxtPtr ctxt) {
    (void)ctxt;
    return (void*)0x2;
}

void xmlSchemaFreeParserCtxt(xmlSchemaParserCtxtPtr ctxt) {
    (void)ctxt;
}

void xmlSchemaFree(xmlSchemaPtr schemas) {
    (void)schemas;
}

int schemasOneTest(const char *filename, const char *instance, const char *err, int options, xmlSchemaPtr schemas) {
    (void)filename; (void)instance; (void)err; (void)options; (void)schemas;
    return 0;
}

/* Reimplementation of the vulnerable function focusing on the buggy code path */
int schemasTest(const char *filename, const char *resul, const char *errr, int options) {
    (void)resul; (void)errr; (void)options;

    const char *base = baseFilename(filename);
    xmlSchemaParserCtxtPtr ctxt;
    xmlSchemaPtr schemas;
    int len;

    /* Setup stubs as in original code */
    ctxt = xmlSchemaNewParserCtxt(filename);
    xmlSchemaSetParserStructuredErrors(ctxt, testStructuredErrorHandler, NULL);
    schemas = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);
    (void)schemas; /* Not used further in this minimal reproducer */

    /* Vulnerable logic */
    len = (int)strlen(base);
    if ((len > 499) || (len < 5)) {
        xmlSchemaFree(schemas);
        return -1;
    }
    len -= 4; /* remove trailing .xsd */

    /* Out-of-bounds read when base is of minimal allowed length (e.g., "a.xsd").
       For len == 1, base[len - 2] becomes base[-1]. Use volatile to ensure the read happens. */
    volatile char c = base[len - 2];
    (void)c;

    /* Not necessary to continue; the bug is already triggered. */
    xmlSchemaFree(schemas);
    return 0;
}

int main(void) {
    /* Allocate the minimal filename that passes the (len < 5) check but
       causes len -= 4 to produce len == 1. */
    const char *fname = "a.xsd"; /* length 5 */

    /* Place the string at the start of a heap allocation so that reading base[-1]
       hits the ASan left redzone and reports a heap-buffer-underflow. */
    size_t n = strlen(fname) + 1; /* include NUL */
    char *heap_fname = (char*)malloc(n);
    if (!heap_fname) {
        perror("malloc");
        return 1;
    }
    memcpy(heap_fname, fname, n);

    /* This call will perform base[len - 2] with len == 1, i.e., base[-1]. */
    (void)schemasTest(heap_fname, NULL, NULL, 0);

    /* If ASan didn't abort (it should), clean up. */
    free(heap_fname);
    return 0;
}
