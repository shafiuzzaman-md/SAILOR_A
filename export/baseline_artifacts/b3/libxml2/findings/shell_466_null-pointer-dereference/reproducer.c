#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal libxml-like type and macro definitions */
typedef unsigned char xmlChar;

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

#define XML_ELEMENT_NODE 1
#define XML_ATTRIBUTE_NODE 2
#define XML_TEXT_NODE 3
#define XML_ENTITY_REF_NODE 5
#define XML_COMMENT_NODE 8
#define XML_DOCUMENT_NODE 9
#define XML_HTML_DOCUMENT_NODE 13

struct _xmlNode;

typedef struct _xmlDoc {
    struct _xmlNode *children;
} *xmlDocPtr;

typedef struct _xmlNode {
    struct _xmlNode *next;
    struct _xmlNode *prev;
    struct _xmlNode *parent;
    struct _xmlNode *children;
    unsigned short type;
    xmlChar *name;
    xmlChar *content;
} xmlNode, *xmlNodePtr;

/* Shell context holding an output FILE* like xmllint */
typedef struct _xmllintShellCtxt {
    FILE *output;
} xmllintShellCtxt, *xmllintShellCtxtPtr;

/* Stubs for functions used by xmllintShellGrep */
static xmlChar *xmlGetNodePath(xmlNodePtr node ATTRIBUTE_UNUSED) {
    return (xmlChar*)"/fake/path";
}

static int xmllintShellList(xmllintShellCtxtPtr ctxt ATTRIBUTE_UNUSED,
                            char *arg ATTRIBUTE_UNUSED,
                            xmlNodePtr node ATTRIBUTE_UNUSED,
                            xmlNodePtr node2 ATTRIBUTE_UNUSED) {
    /* No-op stub */
    return 0;
}

/* Minimal xmlStrstr that dereferences the haystack, so passing NULL crashes */
static const xmlChar *xmlStrstr(const xmlChar *haystack, const xmlChar *needle) {
    if (needle == NULL)
        return NULL;
    /* This loop will immediately dereference 'haystack' (NULL in our trigger) */
    for (const xmlChar *h = haystack; *h; h++) {
        const xmlChar *h2 = h;
        const xmlChar *n = needle;
        while (*h2 && *n && (*h2 == *n)) {
            h2++; n++;
        }
        if (*n == 0)
            return h;
    }
    return NULL;
}

/* Vulnerable function copied and minimally adapted */
static int
xmllintShellGrep(xmllintShellCtxtPtr ctxt ATTRIBUTE_UNUSED,
            char *arg, xmlNodePtr node, xmlNodePtr node2 ATTRIBUTE_UNUSED)
{
    if (!ctxt)
        return (0);
    if (node == NULL)
        return (0);
    if (arg == NULL)
        return (0);
#ifdef LIBXML_REGEXP_ENABLED
    if ((xmlStrchr((xmlChar *) arg, '?')) ||
        (xmlStrchr((xmlChar *) arg, '*')) ||
        (xmlStrchr((xmlChar *) arg, '.')) ||
        (xmlStrchr((xmlChar *) arg, '['))) {
    }
#endif
    while (node != NULL) {
        if (node->type == XML_COMMENT_NODE) {
            if (xmlStrstr(node->content, (xmlChar *) arg)) {
                fprintf(ctxt->output, "%s : ", (char*)xmlGetNodePath(node));
                xmllintShellList(ctxt, NULL, node, NULL);
            }
        } else if (node->type == XML_TEXT_NODE) {
            /* Vulnerable line: node->content may be NULL for a text node */
            if (xmlStrstr(node->content, (xmlChar *) arg)) {
                fprintf(ctxt->output, "%s : ", (char*)xmlGetNodePath(node->parent));
                xmllintShellList(ctxt, NULL, node->parent, NULL);
            }
        }

        /* Browse the full subtree, deep first */
        if ((node->type == XML_DOCUMENT_NODE) ||
            (node->type == XML_HTML_DOCUMENT_NODE)) {
            node = ((xmlDocPtr) node)->children;
        } else if ((node->children != NULL)
                   && (node->type != XML_ENTITY_REF_NODE)) {
            /* deep first */
            node = node->children;
        } else if (node->next != NULL) {
            /* then siblings */
            node = node->next;
        } else {
            /* go up to parents->next if needed */
            while (node != NULL) {
                if (node->parent != NULL) {
                    node = node->parent;
                }
                if (node->next != NULL) {
                    node = node->next;
                    break;
                }
                if (node->parent == NULL) {
                    node = NULL;
                    break;
                }
            }
        }
    }
    return (0);
}

int main(void) {
    /* Set up a shell context with a valid output stream */
    xmllintShellCtxt ctxt;
    ctxt.output = stdout;

    /* Create a parent element node */
    xmlNodePtr parent = (xmlNodePtr)calloc(1, sizeof(xmlNode));
    if (!parent) return 1;
    parent->type = XML_ELEMENT_NODE;

    /* Create a text node with NULL content to trigger the bug */
    xmlNodePtr text = (xmlNodePtr)calloc(1, sizeof(xmlNode));
    if (!text) return 1;
    text->type = XML_TEXT_NODE;
    text->content = NULL;           /* Critical: NULL content */
    text->parent = parent;          /* So xmlGetNodePath(parent) is reachable */
    parent->children = text;

    /* Non-NULL search argument */
    char arg[] = "needle";

    /* Call the vulnerable function: this will dereference NULL via xmlStrstr */
    xmllintShellGrep(&ctxt, arg, text, NULL);

    /* Cleanup (never reached if crash occurs) */
    free(text);
    free(parent);
    return 0;
}
