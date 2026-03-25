#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

/* Minimal type stubs to avoid depending on libxml2 headers */
typedef struct _xmlDoc xmlDoc;
typedef xmlDoc *xmlDocPtr;

typedef struct _xmlNode xmlNode;
typedef xmlNode *xmlNodePtr;

typedef struct _xmlRelaxNGParserCtxt xmlRelaxNGParserCtxt;
typedef xmlRelaxNGParserCtxt *xmlRelaxNGParserCtxtPtr;

typedef struct _xmlRelaxNG xmlRelaxNG;
typedef xmlRelaxNG *xmlRelaxNGPtr;

typedef struct _xmlRelaxNGValidCtxt xmlRelaxNGValidCtxt;
typedef xmlRelaxNGValidCtxt *xmlRelaxNGValidCtxtPtr;

/* Callback typedefs (match libxml2 style) */
typedef void (*xmlRelaxNGValidityErrorFunc)(void *ctx, const char *msg, ...);

/* Shell context used by xmllint */
typedef struct _xmllintShellCtxt {
    FILE *output;
    xmlDocPtr doc;
    const char *filename;
} xmllintShellCtxt, *xmllintShellCtxtPtr;

/* --- Stub implementations of the libxml2 RelaxNG API used by the shell --- */
/* Simulate allocation failure: return NULL to trigger the vulnerable path */
xmlRelaxNGParserCtxtPtr xmlRelaxNGNewParserCtxt(const char *URLorFile) {
    (void)URLorFile;
    return NULL; /* Emulate OOM/internal error */
}

/* This function will dereference the parser context unconditionally,
 * mimicking the behavior that crashes when ctxt is NULL. */
void xmlRelaxNGSetParserErrors(xmlRelaxNGParserCtxtPtr ctxt,
                               xmlRelaxNGValidityErrorFunc err,
                               xmlRelaxNGValidityErrorFunc warn,
                               void *ctx) {
    (void)err; (void)warn; (void)ctx;
    /* Intentional NULL dereference when ctxt == NULL to demonstrate the bug */
    volatile unsigned char *p = (unsigned char *)ctxt;
    volatile unsigned char val = *p; /* ASan will flag this when ctxt==NULL */
    (void)val;
}

/* The remaining API calls are never reached due to the crash but are provided
 * to make the file self-contained and linkable regardless. */
xmlRelaxNGPtr xmlRelaxNGParse(xmlRelaxNGParserCtxtPtr ctxt) {
    (void)ctxt; return NULL;
}
void xmlRelaxNGFreeParserCtxt(xmlRelaxNGParserCtxtPtr ctxt) {
    (void)ctxt;
}
xmlRelaxNGValidCtxtPtr xmlRelaxNGNewValidCtxt(xmlRelaxNGPtr schema) {
    (void)schema; return NULL;
}
void xmlRelaxNGSetValidErrors(xmlRelaxNGValidCtxtPtr vctxt,
                              xmlRelaxNGValidityErrorFunc err,
                              xmlRelaxNGValidityErrorFunc warn,
                              void *ctx) {
    (void)vctxt; (void)err; (void)warn; (void)ctx;
}
int xmlRelaxNGValidateDoc(xmlRelaxNGValidCtxtPtr vctxt, xmlDocPtr doc) {
    (void)vctxt; (void)doc; return -1;
}
void xmlRelaxNGFreeValidCtxt(xmlRelaxNGValidCtxtPtr vctxt) {
    (void)vctxt;
}
void xmlRelaxNGFree(xmlRelaxNGPtr schema) {
    (void)schema;
}

/* The shell's printf callback used for error/warning reporting */
static void xmllintShellPrintf(void *ctx, const char *msg, ...) {
    xmllintShellCtxtPtr sctxt = (xmllintShellCtxtPtr)ctx;
    va_list ap;
    va_start(ap, msg);
    vfprintf(sctxt->output, msg, ap);
    va_end(ap);
}

/* Reimplementation of the vulnerable function from shell.c */
static int xmllintShellRNGValidate(xmllintShellCtxtPtr sctxt, char *schemas,
                                   xmlNodePtr node ATTRIBUTE_UNUSED,
                                   xmlNodePtr node2 ATTRIBUTE_UNUSED) {
    xmlRelaxNGPtr relaxngschemas;
    xmlRelaxNGParserCtxtPtr ctxt;
    xmlRelaxNGValidCtxtPtr vctxt;
    int ret;

    (void)node; (void)node2;

    /* Returns NULL in our stub to simulate allocation failure */
    ctxt = xmlRelaxNGNewParserCtxt(schemas);

    /* Vulnerable call: dereferences ctxt without NULL check */
    xmlRelaxNGSetParserErrors(ctxt, xmllintShellPrintf, xmllintShellPrintf, sctxt);

    /* Unreached code below (kept for fidelity) */
    relaxngschemas = xmlRelaxNGParse(ctxt);
    xmlRelaxNGFreeParserCtxt(ctxt);
    if (relaxngschemas == NULL) {
        fprintf(sctxt->output, "Relax-NG schema %s failed to compile\n", schemas);
        return -1;
    }
    vctxt = xmlRelaxNGNewValidCtxt(relaxngschemas);
    xmlRelaxNGSetValidErrors(vctxt, xmllintShellPrintf, xmllintShellPrintf, sctxt);
    ret = xmlRelaxNGValidateDoc(vctxt, sctxt->doc);
    if (ret == 0) {
        fprintf(sctxt->output, "%s validates\n", sctxt->filename);
    } else if (ret > 0) {
        fprintf(sctxt->output, "%s fails to validate\n", sctxt->filename);
    } else {
        fprintf(sctxt->output, "%s validation generated an internal error\n", sctxt->filename);
    }
    xmlRelaxNGFreeValidCtxt(vctxt);
    if (relaxngschemas != NULL)
        xmlRelaxNGFree(relaxngschemas);
    return 0;
}

int main(void) {
    xmllintShellCtxt sctx;
    sctx.output = stdout;
    sctx.doc = NULL;            /* Not used before the crash */
    sctx.filename = "test.xml"; /* For messages only */

    char schemas[] = "dummy.rng";

    /* This call will reach the vulnerable line and crash via NULL deref */
    (void)xmllintShellRNGValidate(&sctx, schemas, NULL, NULL);

    /* Should be unreachable */
    fprintf(stdout, "If you see this, the bug did not trigger.\n");
    return 0;
}
