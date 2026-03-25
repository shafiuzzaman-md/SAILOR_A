#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for libxml2 types used by the vulnerable code */
typedef void* xmlNodePtr;

typedef struct _xmlRelaxNGParserCtxt { int dummy; } *xmlRelaxNGParserCtxtPtr;
typedef struct _xmlRelaxNGValidCtxt  { int dummy; } *xmlRelaxNGValidCtxtPtr;
typedef struct _xmlRelaxNG           { int dummy; } *xmlRelaxNGPtr;

/* Shell context as used by xmllint shell code */
typedef struct _xmllintShellCtxt {
    FILE *output;
    const char *filename;
    void *doc;
} xmllintShellCtxt, *xmllintShellCtxtPtr;

/* Callback used by the shell for printing */
static void xmllintShellPrintf(void *ctx, const char *msg, ...) {
    xmllintShellCtxtPtr sctxt = (xmllintShellCtxtPtr)ctx;
    va_list ap;
    va_start(ap, msg);
    vfprintf(sctxt->output, msg, ap);
    va_end(ap);
}

/* Stubs simulating the Relax NG API (enough to reach the bug) */
static xmlRelaxNGParserCtxtPtr xmlRelaxNGNewParserCtxt(const char *schemas) {
    (void)schemas;
    /* return a non-NULL parser context */
    return (xmlRelaxNGParserCtxtPtr)malloc(sizeof(struct _xmlRelaxNGParserCtxt));
}

static void xmlRelaxNGSetParserErrors(xmlRelaxNGParserCtxtPtr ctxt,
                                      void (*serror)(void*, const char*, ...),
                                      void (*swarn)(void*, const char*, ...),
                                      void *ctx) {
    (void)ctxt; (void)serror; (void)swarn; (void)ctx;
}

static xmlRelaxNGPtr xmlRelaxNGParse(xmlRelaxNGParserCtxtPtr ctxt) {
    (void)ctxt;
    /* return a non-NULL compiled schema */
    return (xmlRelaxNGPtr)malloc(sizeof(struct _xmlRelaxNG));
}

static void xmlRelaxNGFreeParserCtxt(xmlRelaxNGParserCtxtPtr ctxt) {
    free(ctxt);
}

static xmlRelaxNGValidCtxtPtr xmlRelaxNGNewValidCtxt(xmlRelaxNGPtr schema) {
    (void)schema;
    /* Simulate allocation failure: this returns NULL in the real library on OOM */
    return NULL;
}

/* This function will dereference vctxt, reproducing the NULL deref when called with NULL */
static void xmlRelaxNGSetValidErrors(xmlRelaxNGValidCtxtPtr vctxt,
                                     void (*serror)(void*, const char*, ...),
                                     void (*swarn)(void*, const char*, ...),
                                     void *ctx) {
    (void)serror; (void)swarn; (void)ctx;
    volatile int crash = ((struct _xmlRelaxNGValidCtxt*)vctxt)->dummy; /* NULL deref here */
    (void)crash;
}

static int xmlRelaxNGValidateDoc(xmlRelaxNGValidCtxtPtr vctxt, void *doc) {
    (void)vctxt; (void)doc;
    return 0;
}

static void xmlRelaxNGFreeValidCtxt(xmlRelaxNGValidCtxtPtr vctxt) {
    (void)vctxt;
}

static void xmlRelaxNGFree(xmlRelaxNGPtr schema) {
    free(schema);
}

/* Vulnerable function, adapted from shell.c */
static int xmllintShellRNGValidate(xmllintShellCtxtPtr sctxt, char *schemas,
                                   xmlNodePtr node, xmlNodePtr node2) {
    (void)node; (void)node2;
    xmlRelaxNGPtr relaxngschemas;
    xmlRelaxNGParserCtxtPtr ctxt;
    xmlRelaxNGValidCtxtPtr vctxt;
    int ret;

    ctxt = xmlRelaxNGNewParserCtxt(schemas);
    xmlRelaxNGSetParserErrors(ctxt, xmllintShellPrintf, xmllintShellPrintf, sctxt);
    relaxngschemas = xmlRelaxNGParse(ctxt);
    xmlRelaxNGFreeParserCtxt(ctxt);
    if (relaxngschemas == NULL) {
        fprintf(sctxt->output, "Relax-NG schema %s failed to compile\n", schemas);
        return -1;
    }
    vctxt = xmlRelaxNGNewValidCtxt(relaxngschemas);
    /* BUG: vctxt may be NULL but is passed unchecked to xmlRelaxNGSetValidErrors */
    xmlRelaxNGSetValidErrors(vctxt, xmllintShellPrintf, xmllintShellPrintf, sctxt);

    /* The following is not reached due to the crash above */
    ret = xmlRelaxNGValidateDoc(vctxt, sctxt->doc);
    if (ret == 0) {
        fprintf(sctxt->output, "%s validates\n", sctxt->filename);
    } else if (ret > 0) {
        fprintf(sctxt->output, "%s fails to validate\n", sctxt->filename);
    } else {
        fprintf(sctxt->output, "%s validation generated an internal error\n",
                sctxt->filename);
    }
    xmlRelaxNGFreeValidCtxt(vctxt);
    if (relaxngschemas != NULL)
        xmlRelaxNGFree(relaxngschemas);
    return 0;
}

int main(void) {
    xmllintShellCtxt sctxt;
    memset(&sctxt, 0, sizeof(sctxt));
    sctxt.output = stdout;
    sctxt.filename = "test.xml";
    sctxt.doc = (void*)0x1; /* dummy non-NULL doc pointer */

    char schemas[] = "schema.rng";
    /* Triggers the NULL dereference inside xmlRelaxNGSetValidErrors */
    xmllintShellRNGValidate(&sctxt, schemas, NULL, NULL);
    return 0;
}
