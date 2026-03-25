#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal type re-declarations to compile standalone */
typedef unsigned char xmlChar;

typedef struct _xmlEnumeration xmlEnumeration;
struct _xmlEnumeration { int dummy; };

typedef struct _xmlAttribute *xmlAttributePtr;

typedef int xmlAttributeType;
typedef int xmlAttributeDefault;

typedef struct _xmlDtd xmlDtd;
typedef xmlDtd* xmlDtdPtr;
struct _xmlDtd { int dummy; };

typedef struct _xmlDoc xmlDoc;
typedef xmlDoc* xmlDocPtr;
struct _xmlDoc {
    xmlDtdPtr intSubset;
    xmlDtdPtr extSubset;
};

typedef struct _xmlValidCtxt xmlValidCtxt;
typedef xmlValidCtxt* xmlValidCtxtPtr;
struct _xmlValidCtxt {
    int valid;
};

typedef struct _xmlParserCtxt xmlParserCtxt;
typedef xmlParserCtxt* xmlParserCtxtPtr;
struct _xmlParserCtxt {
    xmlDocPtr myDoc;
    xmlValidCtxt vctxt;
    int inSubset;    /* 1 = internal, 2 = external */
    int validate;
    int wellFormed;
    int valid;
};

#ifndef BAD_CAST
#define BAD_CAST (xmlChar *)
#endif

/* Error/utility stubs */
#define XML_ERR_INTERNAL_ERROR 1
#define XML_DTD_XMLID_TYPE 2

static int xmlStrEqual(const xmlChar *a, const xmlChar *b) {
    if (a == NULL || b == NULL) return 0;
    return strcmp((const char*)a, (const char*)b) == 0;
}

static const xmlChar *xmlSplitQName4(const xmlChar *fullname, xmlChar **prefix) {
    /* Stub which simulates failure and returns NULL to trigger the bug */
    (void)fullname;
    if (prefix) *prefix = NULL;
    return NULL;
}

static void xmlSAX2ErrMemory(xmlParserCtxtPtr ctxt) {
    fprintf(stderr, "xmlSAX2ErrMemory: simulated memory error (ctx=%p)\n", (void*)ctxt);
}

static void xmlFree(void *ptr) {
    free(ptr);
}

static void xmlFreeEnumeration(xmlEnumeration *tree) {
    (void)tree;
}

static void xmlFatalErrMsg(xmlParserCtxtPtr ctxt, int err, const char *msg,
                           const xmlChar *str1, const xmlChar *str2) {
    (void)ctxt; (void)err; (void)str1; (void)str2;
    fprintf(stderr, "xmlFatalErrMsg: %s\n", msg);
}

static void xmlErrId(xmlParserCtxtPtr ctxt, int code, const char *msg, void *unused) {
    (void)ctxt; (void)code; (void)unused;
    fprintf(stderr, "xmlErrId: %s\n", msg);
}

/* This stub intentionally dereferences 'name' to demonstrate the NULL deref
 * propagated from xmlSAX2AttributeDecl when xmlSplitQName4 returns NULL. */
static xmlAttributePtr xmlAddAttributeDecl(xmlValidCtxtPtr vctxt, xmlDtdPtr subset,
        const xmlChar *elem, const xmlChar *name, const xmlChar *prefix,
        xmlAttributeType type, xmlAttributeDefault def,
        const xmlChar *defaultValue, xmlEnumeration *tree) {
    (void)vctxt; (void)subset; (void)elem; (void)prefix; (void)type; (void)def; (void)defaultValue; (void)tree;
    /* This will crash with a NULL pointer dereference when name == NULL */
    volatile unsigned char c = name[0];
    (void)c;
    return (xmlAttributePtr)0x1;
}

#ifdef LIBXML_VALID_ENABLED
static int xmlValidateAttributeDecl(xmlValidCtxtPtr vctxt, xmlDocPtr doc, xmlAttributePtr attr) {
    (void)vctxt; (void)doc; (void)attr;
    return 1;
}
#endif

/* Vulnerable function copied/minimized from SAX2.c */
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

    if ((xmlStrEqual(fullname, BAD_CAST "xml:id")) &&
        (type != 5 /* XML_ATTRIBUTE_ID placeholder */)) {
        xmlErrId(ctxt, XML_DTD_XMLID_TYPE,
              "xml:id : attribute type should be ID\n", NULL);
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
        xmlFatalErrMsg(ctxt, XML_ERR_INTERNAL_ERROR,
             "SAX.xmlSAX2AttributeDecl(%s) called while not in subset\n",
                       name, NULL);
        xmlFree(prefix);
        xmlFreeEnumeration(tree);
        return;
    }
#ifdef LIBXML_VALID_ENABLED
    if (ctxt->vctxt.valid == 0)
        ctxt->valid = 0;
    if ((attr != NULL) && (ctxt->validate) && (ctxt->wellFormed) &&
        (ctxt->myDoc->intSubset != NULL))
        ctxt->valid &= xmlValidateAttributeDecl(&ctxt->vctxt, ctxt->myDoc,
                                                attr);
#endif /* LIBXML_VALID_ENABLED */
    if (prefix != NULL)
        xmlFree(prefix);
}

int main(void) {
    /* Set up a parser context with an external subset (inSubset == 2). */
    xmlDocPtr doc = (xmlDocPtr)calloc(1, sizeof(*doc));
    if (!doc) return 1;
    /* Non-NULL external subset to reach the vulnerable call site */
    doc->extSubset = (xmlDtdPtr)0x1;

    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr)calloc(1, sizeof(*ctxt));
    if (!ctxt) return 1;
    ctxt->myDoc = doc;
    ctxt->inSubset = 2;       /* external subset path */
    ctxt->validate = 0;
    ctxt->wellFormed = 1;
    ctxt->valid = 1;

    /* fullname value is irrelevant since our xmlSplitQName4 stub returns NULL */
    const xmlChar *elem = BAD_CAST "elem";
    const xmlChar *fullname = BAD_CAST "bad:attr";

    /* Trigger the bug: xmlSplitQName4 returns NULL, name remains NULL, and is
     * then passed to xmlAddAttributeDecl, which dereferences it. */
    xmlSAX2AttributeDecl(ctxt, elem, fullname,
                         0 /* type */, 0 /* def */, NULL /* defaultValue */, NULL /* tree */);

    /* Clean up (unreached on crash) */
    free(ctxt);
    free(doc);
    return 0;
}
