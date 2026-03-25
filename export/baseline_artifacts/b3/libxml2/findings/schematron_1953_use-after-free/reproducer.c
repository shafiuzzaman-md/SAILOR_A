#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for libxml2 types */
typedef struct _xmlNode xmlNode;
typedef struct _xmlDoc xmlDoc;
typedef struct _xmlSchematronTest xmlSchematronTest;
typedef struct _xmlSchematronRule xmlSchematronRule;
typedef struct _xmlSchematronSchema xmlSchematronSchema;
typedef struct _xmlSchematronValidCtxt xmlSchematronValidCtxt;

typedef xmlNode* xmlNodePtr;
typedef xmlDoc* xmlDocPtr;
typedef void* xmlSchematronPatternPtr;
typedef xmlSchematronTest* xmlSchematronTestPtr;
typedef xmlSchematronRule* xmlSchematronRulePtr;
typedef xmlSchematronSchema* xmlSchematronSchemaPtr;
typedef xmlSchematronValidCtxt* xmlSchematronValidCtxtPtr;

struct _xmlNode {
    xmlNodePtr parent;
    xmlNodePtr children;
    xmlNodePtr next;
    xmlNodePtr prev;
    xmlDocPtr  doc;
    char      *name;
};

struct _xmlDoc {
    xmlNodePtr root;
};

struct _xmlSchematronTest {
    xmlSchematronTestPtr next;
};

struct _xmlSchematronRule {
    xmlSchematronPatternPtr pattern;
    xmlSchematronTestPtr tests;
    xmlSchematronRulePtr next;   /* used in quiet branch */
    xmlSchematronRulePtr patnext;/* unused here */
};

struct _xmlSchematronSchema {
    xmlSchematronRulePtr rules;
};

struct _xmlSchematronValidCtxt {
    xmlSchematronSchemaPtr schema;
    int flags;
    int nberrors;
    void *xctxt;
};

/* Stubs emulating libxml2 helpers */
static xmlNodePtr xmlDocGetRootElement(xmlDocPtr doc) {
    return doc ? doc->root : NULL;
}

static int xmlPatternMatch(xmlSchematronPatternPtr pattern, xmlNodePtr cur) {
    (void)pattern; (void)cur;
    /* Force-match every node to enter the vulnerable path */
    return 1;
}

static int xmlSchematronRegisterVariables(xmlSchematronValidCtxtPtr ctxt, void *xctxt,
                                          void *lets, xmlDocPtr instance, xmlNodePtr cur) {
    (void)ctxt; (void)xctxt; (void)lets; (void)instance; (void)cur;
    return 0; /* success */
}

static int xmlSchematronUnregisterVariables(xmlSchematronValidCtxtPtr ctxt, void *xctxt,
                                            void *lets) {
    (void)ctxt; (void)xctxt; (void)lets;
    return 0; /* success */
}

/* Global flag to ensure we only free once in this demo */
static int g_freed_node = 0;

/* This simulates xmlSchematronRunTest() calling a user callback that frees the current node */
static void xmlSchematronRunTest(xmlSchematronValidCtxtPtr ctxt, xmlSchematronTestPtr test,
                                 xmlDocPtr instance, xmlNodePtr cur,
                                 xmlSchematronPatternPtr pattern) {
    (void)ctxt; (void)test; (void)instance; (void)pattern;
    if (!g_freed_node && cur) {
        /* Simulate user callback (e.g., xmlRaiseError) freeing the current node */
        /* Intentionally leak name to avoid double-free complexity */
        free(cur);
        g_freed_node = 1;
        /* After returning, caller will continue and use 'cur' assuming it's valid */
    }
}

/* Next-node traversal like libxml2's xmlSchematronNextNode: will dereference 'cur' */
static xmlNodePtr xmlSchematronNextNode(xmlNodePtr cur) {
    /* This function intentionally dereferences fields of 'cur' to trigger UAF */
    if (cur == NULL)
        return NULL;
    if (cur->children != NULL) /* UAF read if 'cur' was freed */
        return cur->children;
    while (cur != NULL) {
        if (cur->next != NULL)
            return cur->next;
        cur = cur->parent;
    }
    return NULL;
}

/* Vulnerable function modeled after libxml2/schematron.c (quiet branch) */
static int xmlSchematronValidateDoc(xmlSchematronValidCtxtPtr ctxt, xmlDocPtr instance) {
    xmlNodePtr root, cur;
    xmlSchematronRulePtr rule;
    xmlSchematronTestPtr test;

    if ((ctxt == NULL) || (ctxt->schema == NULL) ||
        (ctxt->schema->rules == NULL) || (instance == NULL))
        return -1;

    ctxt->nberrors = 0;
    root = xmlDocGetRootElement(instance);
    if (root == NULL) {
        ctxt->nberrors++;
        return 1;
    }

    /* Quiet/fast path */
    cur = root;
    while (cur != NULL) {
        rule = ctxt->schema->rules;
        while (rule != NULL) {
            if (xmlPatternMatch(rule->pattern, cur) == 1) {
                test = rule->tests;

                if (xmlSchematronRegisterVariables(ctxt, ctxt->xctxt,
                                                   NULL, instance, cur))
                    return -1;

                while (test != NULL) {
                    /* This will free 'cur' via a simulated callback */
                    xmlSchematronRunTest(ctxt, test, instance, cur, rule->pattern);
                    test = test->next;
                }

                if (xmlSchematronUnregisterVariables(ctxt, ctxt->xctxt, NULL))
                    return -1;
            }
            rule = rule->next;
        }

        /* Use-after-free: 'cur' may have been freed by the callback above */
        cur = xmlSchematronNextNode(cur);
    }

    return 0;
}

int main(void) {
    /* Build a minimal document tree with a single root node */
    xmlDocPtr doc = (xmlDocPtr)malloc(sizeof(xmlDoc));
    memset(doc, 0, sizeof(*doc));

    xmlNodePtr root = (xmlNodePtr)malloc(sizeof(xmlNode));
    memset(root, 0, sizeof(*root));
    root->name = NULL; /* not used */
    root->doc = doc;
    doc->root = root;

    /* Build a schema with one rule and one test so we enter RunTest */
    xmlSchematronTestPtr t1 = (xmlSchematronTestPtr)malloc(sizeof(xmlSchematronTest));
    memset(t1, 0, sizeof(*t1));

    xmlSchematronRulePtr r1 = (xmlSchematronRulePtr)malloc(sizeof(xmlSchematronRule));
    memset(r1, 0, sizeof(*r1));
    r1->tests = t1;  /* one test */

    xmlSchematronSchemaPtr schema = (xmlSchematronSchemaPtr)malloc(sizeof(xmlSchematronSchema));
    memset(schema, 0, sizeof(*schema));
    schema->rules = r1;

    xmlSchematronValidCtxt ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.schema = schema;
    ctx.flags = 0; /* ensure we go through the quiet/fast path */

    /* Trigger validation: during the first test, the current node is freed.
       On return, xmlSchematronNextNode(cur) dereferences the freed 'cur'. */
    int ret = xmlSchematronValidateDoc(&ctx, doc);

    /* Cleanup what remains (in practice, we likely crashed/ASAN before here) */
    free(t1);
    free(r1);
    free(schema);
    free(doc); /* root was already freed by the simulated callback */

    return ret == 0 ? 0 : 1;
}
