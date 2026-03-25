#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type re-declarations to simulate the libxml2 shell context and XPath API */
typedef unsigned char xmlChar;

typedef struct _xmlNode xmlNode;
typedef xmlNode* xmlNodePtr;
typedef struct _xmlDoc xmlDoc;
typedef xmlDoc* xmlDocPtr;

typedef struct _xmlXPathContext {
    xmlNodePtr node;
} xmlXPathContext;
typedef xmlXPathContext* xmlXPathContextPtr;

typedef enum {
    XPATH_UNDEFINED = 0,
    XPATH_NODESET = 1
} xmlXPathObjectType;

typedef struct _xmlXPathObject {
    xmlXPathObjectType type;
} xmlXPathObject;
typedef xmlXPathObject* xmlXPathObjectPtr;

/* Shell context like xmllintShellCtxt */
typedef struct {
    FILE *output;
    xmlDocPtr doc;
    xmlNodePtr node;
    xmlXPathContextPtr pctxt;
} xmllintShellCtxt;

/* Stub: xmlXPathEval returns NULL for invalid expressions.
 * For the purpose of this reproducer, we always return NULL to simulate
 * an invalid XPath expression. */
xmlXPathObjectPtr xmlXPathEval(const xmlChar *expr, xmlXPathContextPtr ctx) {
    (void)expr;
    (void)ctx;
    return NULL; /* Simulate parse/eval failure */
}

/* Stub: xmlXPathDebugDumpObject wrongly assumes 'cur' is non-NULL and dereferences it. */
void xmlXPathDebugDumpObject(FILE *output, xmlXPathObjectPtr cur, int depth) {
    (void)depth;
    /* This dereference will crash when cur == NULL, simulating the NPD in the real code */
    fprintf(output, "Object type: %d\n", cur->type);
}

/* Stub: free function is a no-op, tolerates NULL. */
void xmlXPathFreeObject(xmlXPathObjectPtr obj) {
    (void)obj;
}

/* Minimal reproduction of the vulnerable branch inside xmllintShell */
void xmllintShell(xmllintShellCtxt *ctxt, const char *command, const char *arg) {
    if (!strcmp(command, "xpath")) {
        if (arg[0] == '\0') {
            fprintf(ctxt->output, "xpath: expression required\n");
            return;
        }
        /* Vulnerable sequence: no NULL check on the result of xmlXPathEval */
        ctxt->pctxt->node = ctxt->node;
        xmlXPathObjectPtr list = xmlXPathEval((const xmlChar *)arg, ctxt->pctxt);
        /* BUG: list may be NULL. The following call dereferences it unconditionally. */
        xmlXPathDebugDumpObject(ctxt->output, list, 0);
        xmlXPathFreeObject(list);
    } else {
        fprintf(ctxt->output, "Unknown command: %s\n", command);
    }
}

int main(void) {
    /* Set up a shell context similar to xmllint's shell */
    xmllintShellCtxt ctxt;
    memset(&ctxt, 0, sizeof(ctxt));
    ctxt.output = stdout;

    /* Allocate a minimal XPath context */
    ctxt.pctxt = (xmlXPathContextPtr)malloc(sizeof(xmlXPathContext));
    if (!ctxt.pctxt) {
        fprintf(stderr, "Allocation failure\n");
        return 1;
    }
    ctxt.pctxt->node = NULL;

    /* Dummy doc/node pointers (not used by stubs) */
    ctxt.doc = (xmlDocPtr)0x1;
    ctxt.node = (xmlNodePtr)0x1;

    /* Pass a non-empty but invalid XPath expression to trigger xmlXPathEval failure (NULL) */
    const char *invalid_xpath = "("; /* Invalid XPath expression */

    /* This will hit the vulnerable path and crash due to NULL deref inside xmlXPathDebugDumpObject */
    xmllintShell(&ctxt, "xpath", invalid_xpath);

    free(ctxt.pctxt);
    return 0;
}
