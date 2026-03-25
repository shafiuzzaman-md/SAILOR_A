#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal type defs to mirror libxml2 schematron internals for this reproducer */
typedef unsigned char xmlChar;

typedef struct _xmlSchematronLet xmlSchematronLet;
typedef xmlSchematronLet *xmlSchematronLetPtr;

struct _xmlSchematronLet {
    xmlChar *name;
    void *comp;          /* pretend this is an xmlXPathCompExprPtr */
    xmlSchematronLetPtr next;
};

typedef struct _xmlSchematronRule {
    xmlSchematronLetPtr lets;
} xmlSchematronRule, *xmlSchematronRulePtr;

typedef struct _xmlSchematronParserCtxt {
    void *xctxt;  /* pretend this is an xmlXPathContextPtr */
} xmlSchematronParserCtxt, *xmlSchematronParserCtxtPtr;

/* libxml2 uses a global function pointer named xmlMalloc; we emulate it here */
typedef void *(*xmlMallocFunc)(size_t size);
static void *failingMalloc(size_t size) {
    (void)size;
    /* Simulate OOM exactly where the bug occurs */
    return NULL;
}
static xmlMallocFunc xmlMalloc = failingMalloc;

/* Stubs for functions referenced in surrounding code */
static void xmlSchematronPErr(xmlSchematronParserCtxtPtr ctxt, void *cur,
                              int err, const char *msg,
                              const char *str1, const char *str2) {
    (void)ctxt; (void)cur; (void)err; (void)str1; (void)str2;
    fprintf(stderr, "xmlSchematronPErr: %s\n", msg ? msg : "(null)");
}

static void *xmlXPathCtxtCompile(void *xctxt, const xmlChar *expr) {
    (void)xctxt; (void)expr;
    /* Return a non-NULL compiled expression to pass the preceding checks */
    return (void*)0x1;
}

static void xmlFree(void *ptr) {
    free(ptr);
}

/* Reimplementation of the vulnerable snippet from xmlSchematronParseRule */
static void xmlSchematronParseRule(xmlSchematronParserCtxtPtr ctxt,
                                   void *cur,
                                   xmlSchematronRulePtr ruleptr) {
    /* Pretend the parser already validated name/value and compiled expression */
    xmlChar *name = (xmlChar*)strdup("letVar");
    xmlChar *value = (xmlChar*)strdup("1");
    void *var_comp = xmlXPathCtxtCompile(ctxt->xctxt, value);
    if (name == NULL || value == NULL || var_comp == NULL) {
        fprintf(stderr, "Setup failure (unexpected)\n");
        exit(1);
    }

    /* Vulnerable allocation + immediate dereference without NULL check */
    xmlSchematronLetPtr let = (xmlSchematronLetPtr) xmlMalloc(sizeof(xmlSchematronLet));

    /* The following line dereferences 'let' without checking for NULL.
       Since our xmlMalloc always returns NULL, this triggers a NULL-deref. */
    let->name = name;            /* <-- crash here (mirrors line 1008) */
    let->comp = var_comp;        /* line 1009 in context */
    let->next = NULL;            /* line 1010 in context */

    /* Normally the list insertion would happen here */
    if (ruleptr->lets != NULL) {
        let->next = ruleptr->lets;
    }
    ruleptr->lets = let;

    xmlFree(value);
}

int main(void) {
    /* Set up minimal structures to reach the vulnerable code path */
    xmlSchematronParserCtxtPtr ctxt = (xmlSchematronParserCtxtPtr)calloc(1, sizeof(*ctxt));
    xmlSchematronRulePtr rule = (xmlSchematronRulePtr)calloc(1, sizeof(*rule));
    if (!ctxt || !rule) {
        fprintf(stderr, "Failed to allocate initial structures\n");
        return 1;
    }
    ctxt->xctxt = (void*)0x2; /* non-NULL to satisfy compile stub */

    /* This call will crash with ASan reporting a NULL pointer dereference */
    xmlSchematronParseRule(ctxt, NULL, rule);

    /* Should never reach here */
    fprintf(stderr, "Unexpectedly returned from xmlSchematronParseRule\n");
    return 0;
}
