#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal libxml2-like type definitions to reproduce the bug */
typedef unsigned char xmlChar;

typedef enum {
    XML_ELEMENT_NODE = 1,
    XML_DOCUMENT_NODE = 9,
    XML_HTML_DOCUMENT_NODE = 13,
    XML_NAMESPACE_DECL = 18
} xmlElementType;

typedef struct _xmlNs {
    const xmlChar *href;
} xmlNs, *xmlNsPtr;

typedef struct _xmlNode {
    void *_private;
    xmlElementType type;
    const xmlChar *name;
    struct _xmlNode *children;
    struct _xmlNode *last;
    struct _xmlNode *parent;
    struct _xmlNode *next;
    struct _xmlNode *prev;
    xmlNsPtr ns;
} xmlNode, *xmlNodePtr;

typedef enum {
    XML_OP_END = 0,
    XML_OP_ROOT = 1,
    XML_OP_ELEM = 2,
    XML_OP_CHILD = 3
} xmlStepOpType;

typedef struct _xmlStepOp {
    int op;
    const xmlChar *value;
    const xmlChar *value2;
} xmlStepOp, *xmlStepOpPtr;

typedef struct _xmlPattern {
    int nbStep;
    xmlStepOp *steps;
} xmlPattern, *xmlPatternPtr;

/* Stub for xmlStrEqual to satisfy references if needed elsewhere */
static int xmlStrEqual(const xmlChar *a, const xmlChar *b) {
    if (a == b) return 1;
    if (a == NULL || b == NULL) return 0;
    const unsigned char *pa = a, *pb = b;
    while (*pa && *pb) {
        if (*pa != *pb) return 0;
        pa++; pb++;
    }
    return (*pa == 0 && *pb == 0);
}

/* Vulnerable function reimplemented to mirror the problematic logic */
static int xmlPatMatch(xmlPatternPtr comp, xmlNodePtr node) {
    if ((comp == NULL) || (node == NULL)) return -1;

    for (int i = 0; i < comp->nbStep; i++) {
        xmlStepOpPtr step = &comp->steps[i];
        switch (step->op) {
            case XML_OP_ROOT:
                /* First checks current node type */
                if (node->type == XML_NAMESPACE_DECL)
                    return 0; /* would be rollback in real code */
                /* Move to parent (which may be NULL for a detached node) */
                node = node->parent;
                /* BUG: node may now be NULL, but code dereferences node->type */
                if ((node->type == XML_DOCUMENT_NODE) ||
                    (node->type == XML_HTML_DOCUMENT_NODE))
                    continue;
                return 0; /* rollback */
            default:
                /* Not needed for this reproducer */
                break;
        }
    }
    return 0;
}

int main(void) {
    /* Create a detached element node (parent == NULL) */
    xmlNodePtr leaf = (xmlNodePtr)calloc(1, sizeof(xmlNode));
    if (!leaf) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    leaf->type = XML_ELEMENT_NODE;
    leaf->parent = NULL; /* Detached node triggers the bug on ROOT step */

    /* Create a compiled pattern with a single ROOT step */
    xmlPatternPtr pat = (xmlPatternPtr)calloc(1, sizeof(xmlPattern));
    if (!pat) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    pat->nbStep = 1;
    pat->steps = (xmlStepOp *)calloc(1, sizeof(xmlStepOp));
    if (!pat->steps) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    pat->steps[0].op = XML_OP_ROOT;
    pat->steps[0].value = NULL;
    pat->steps[0].value2 = NULL;

    /* This call will NULL-dereference inside XML_OP_ROOT handling */
    /* Specifically at: if ((node->type == XML_DOCUMENT_NODE) || ...) */
    int ret = xmlPatMatch(pat, leaf);

    /* Should not reach here if ASan catches the crash */
    printf("xmlPatMatch returned %d\n", ret);

    free(pat->steps);
    free(pat);
    free(leaf);
    return 0;
}
