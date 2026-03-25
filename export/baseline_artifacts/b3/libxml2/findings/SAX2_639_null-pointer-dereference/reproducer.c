#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal libxml2-like types and macros */
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

typedef struct _xmlEnumeration xmlEnumeration; /* unused in this reproducer */
typedef void* xmlAttributePtr;                /* placeholder */
typedef void* xmlDtdPtr;                      /* placeholder */
typedef int xmlAttributeType;                 /* placeholder */
typedef int xmlAttributeDefault;              /* placeholder */

typedef struct _xmlValidCtxt {
    int valid;
} xmlValidCtxt, *xmlValidCtxtPtr;

typedef struct _xmlDoc {
    xmlDtdPtr intSubset;
    xmlDtdPtr extSubset;
} xmlDoc, *xmlDocPtr;

typedef struct _xmlParserCtxt {
    xmlDocPtr myDoc;
    int inSubset;      /* 0: none, 1: internal, 2: external */
    int validate;
    int wellFormed;
    int valid;
    xmlValidCtxt vctxt;
} xmlParserCtxt, *xmlParserCtxtPtr;

/* Constants used by the code path */
enum { XML_ATTRIBUTE_ID = 1 };

typedef enum {
    XML_DTD_XMLID_TYPE = 0
} xmlError;

/* Stubs mimicking libxml2 helpers */
static int xmlStrEqual(const xmlChar *str1, const xmlChar *str2) {
    if (str1 == NULL || str2 == NULL) return 0;
    return strcmp((const char*)str1, (const char*)str2) == 0;
}

static void xmlErrId(xmlParserCtxtPtr ctxt, xmlError err, const char *msg, void *unused) {
    (void)ctxt; (void)err; (void)unused;
    fprintf(stderr, "xmlErrId: %s", msg ? msg : "(null)\n");
}

static void xmlSAX2ErrMemory(xmlParserCtxtPtr ctxt) {
    (void)ctxt;
    fprintf(stderr, "xmlSAX2ErrMemory: out of memory simulated in xmlSplitQName4\n");
}

static void xmlFatalErrMsg(xmlParserCtxtPtr ctxt, int code, const char *msg, const xmlChar *name, void *unused) {
    (void)ctxt; (void)code; (void)unused;
    fprintf(stderr, "xmlFatalErrMsg: ");
    fprintf(stderr, msg ? msg : "(null)", name ? (const char*)name : "(null)");
    fprintf(stderr, "\n");
}

static void xmlFree(void *p) { free(p); }
static void xmlFreeEnumeration(xmlEnumeration *tree) { (void)tree; }

/* This is intentionally implemented to FAIL and return NULL to simulate OOM. */
static const xmlChar *xmlSplitQName4(const xmlChar *fullname, xmlChar **prefix) {
    (void)fullname;
    if (prefix) *prefix = NULL; /* could also set a non-NULL to exercise xmlFree later */
    return NULL; /* simulate failure (e.g., out-of-memory) */
}

/* This stub mimics the bug manifestation by dereferencing 'name' without checking. */
static xmlAttributePtr xmlAddAttributeDecl(xmlValidCtxtPtr vctxt, xmlDtdPtr dtd,
                                           const xmlChar *elem, const xmlChar *name,
                                           xmlChar *prefix, xmlAttributeType type,
                                           xmlAttributeDefault def, const xmlChar *defaultValue,
                                           xmlEnumeration *tree) {
    (void)vctxt; (void)dtd; (void)elem; (void)prefix; (void)type; (void)def; (void)defaultValue; (void)tree;
    /* This will crash with a NULL pointer dereference if 'name' is NULL. */
    size_t n = strlen((const char*)name);
    fprintf(stderr, "xmlAddAttributeDecl: name length = %zu\n", n);
    return NULL;
}

/* Vulnerable function (reproduced from SAX2.c with minimal dependencies) */
void xmlSAX2AttributeDecl(void *ctx, const xmlChar *elem, const xmlChar *fullname,
                          int type, int def, const xmlChar *defaultValue,
                          xmlEnumeration *tree) {
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;
    xmlAttributePtr attr;
    const xmlChar *name = NULL;
    xmlChar *prefix = NULL;

    /* Avoid unused variable warning if features are disabled. */
    (void) attr;

    if ((ctxt == NULL) || (ctxt->myDoc == NULL))
        return;

    if ((xmlStrEqual(fullname, BAD_CAST "xml:id")) && (type != XML_ATTRIBUTE_ID)) {
        xmlErrId(ctxt, XML_DTD_XMLID_TYPE, "xml:id : attribute type should be ID\n", NULL);
    }
    name = xmlSplitQName4(fullname, &prefix);
    if (name == NULL)
        xmlSAX2ErrMemory(ctxt);
    ctxt->vctxt.valid = 1;
    if (ctxt->inSubset == 1)
        attr = xmlAddAttributeDecl(&ctxt->vctxt, ctxt->myDoc->intSubset, elem,
                                   name, prefix, (xmlAttributeType) type,
                                   (xmlAttributeDefault) def, defaultValue, tree);
    else if (ctxt->inSubset == 2)
        attr = xmlAddAttributeDecl(&ctxt->vctxt, ctxt->myDoc->extSubset, elem,
                                   name, prefix, (xmlAttributeType) type,
                                   (xmlAttributeDefault) def, defaultValue, tree);
    else {
        xmlFatalErrMsg(ctxt, 0,
                       "SAX.xmlSAX2AttributeDecl(%s) called while not in subset\n",
                       name, NULL);
        xmlFree(prefix);
        xmlFreeEnumeration(tree);
        return;
    }
#ifndef LIBXML_VALID_ENABLED
    (void)attr; /* suppress unused warning when validation disabled */
#endif
    if (prefix != NULL)
        xmlFree(prefix);
}

int main(void) {
    /* Set up a minimal parser context with an internal subset present */
    xmlDoc doc;
    memset(&doc, 0, sizeof(doc));
    doc.intSubset = (xmlDtdPtr)0x1; /* non-NULL to pass through */

    xmlParserCtxt ctxt;
    memset(&ctxt, 0, sizeof(ctxt));
    ctxt.myDoc = &doc;
    ctxt.inSubset = 1;   /* ensure we take the intSubset branch */
    ctxt.validate = 0;
    ctxt.wellFormed = 1;

    /* fullname can be any QName; failure is simulated by xmlSplitQName4 */
    const xmlChar *elem = BAD_CAST "elem";
    const xmlChar *fullname = BAD_CAST "ns:attr";

    /* Call the vulnerable function: xmlSplitQName4 will return NULL and
       xmlAddAttributeDecl will dereference the NULL 'name' */
    xmlSAX2AttributeDecl(&ctxt, elem, fullname,
                         /* type */ 0,
                         /* def  */ 0,
                         /* defaultValue */ NULL,
                         /* tree */ NULL);

    /* If we get here without crashing (we shouldn't), indicate failure */
    fprintf(stderr, "Unexpectedly survived null deref path.\n");
    return 0;
}
