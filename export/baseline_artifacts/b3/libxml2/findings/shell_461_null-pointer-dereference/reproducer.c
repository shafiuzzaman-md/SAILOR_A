#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal libxml2-like typedefs and enums to reproduce the bug path */
typedef unsigned char xmlChar;

typedef enum {
    XML_ELEMENT_NODE = 1,
    XML_ATTRIBUTE_NODE = 2,
    XML_TEXT_NODE = 3,
    XML_CDATA_SECTION_NODE = 4,
    XML_ENTITY_REF_NODE = 5,
    XML_ENTITY_NODE = 6,
    XML_PI_NODE = 7,
    XML_COMMENT_NODE = 8,
    XML_DOCUMENT_NODE = 9,
    XML_HTML_DOCUMENT_NODE = 13
} xmlElementType;

typedef struct _xmlNode xmlNode;
struct _xmlNode {
    xmlElementType type;
    xmlChar *content;
    xmlNode *children;
    xmlNode *next;
    xmlNode *parent;
};

typedef xmlNode *xmlNodePtr;

typedef struct _xmlDoc {
    xmlNode *children;
} *xmlDocPtr;

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED __attribute__((unused))
#endif

typedef struct _xmllintShellCtxt {
    FILE *output;
    /* other fields are irrelevant for this reproducer */
} *xmllintShellCtxtPtr;

/* Stubs matching libxml2-ish API used by the shell code */
static xmlChar *xmlGetNodePath(xmlNodePtr node ATTRIBUTE_UNUSED) {
    static xmlChar dummy[] = "/dummy/path";
    return dummy;
}

static void xmllintShellList(xmllintShellCtxtPtr ctxt, void *a ATTRIBUTE_UNUSED,
                             xmlNodePtr node ATTRIBUTE_UNUSED, void *b ATTRIBUTE_UNUSED) {
    fprintf(ctxt->output, "[xmllintShellList stub]\n");
}

/* Naive xmlStrstr implementation that dereferences the first argument without NULL checks. */
static xmlChar *xmlStrstr(const xmlChar *str, const xmlChar *val) {
    /* Intentionally no NULL checks to mirror the crash behavior */
    if (*val == 0) /* deref val (non-NULL in our call) */
        return (xmlChar *)str;
    for (const xmlChar *p = str; *p != 0; p++) { /* deref str here -> NULL deref when str == NULL */
        const xmlChar *q = p;
        const xmlChar *r = val;
        while (*q && *r && *q == *r) {
            q++;
            r++;
        }
        if (*r == 0)
            return (xmlChar *)p;
    }
    return NULL;
}

/* Reimplementation of the vulnerable function from shell.c focusing on the buggy path */
static int xmllintShellGrep(xmllintShellCtxtPtr ctxt ATTRIBUTE_UNUSED,
                            char *arg, xmlNodePtr node, xmlNodePtr node2 ATTRIBUTE_UNUSED) {
    if (!ctxt)
        return 0;
    if (node == NULL)
        return 0;
    if (arg == NULL)
        return 0;

    while (node != NULL) {
        if (node->type == XML_COMMENT_NODE) {
            /* BUG: node->content may be NULL for comment nodes; xmlStrstr dereferences it */
            if (xmlStrstr(node->content, (xmlChar *)arg)) {
                fprintf(ctxt->output, "%s : ", (char *)xmlGetNodePath(node));
                xmllintShellList(ctxt, NULL, node, NULL);
            }
        } else if (node->type == XML_TEXT_NODE) {
            if (xmlStrstr(node->content, (xmlChar *)arg)) {
                fprintf(ctxt->output, "%s : ", (char *)xmlGetNodePath(node->parent));
                xmllintShellList(ctxt, NULL, node->parent, NULL);
            }
        }

        /* Minimal traversal logic from the original */
        if ((node->type == XML_DOCUMENT_NODE) ||
            (node->type == XML_HTML_DOCUMENT_NODE)) {
            node = ((xmlDocPtr) node)->children;
        } else if ((node->children != NULL) && (node->type != XML_ENTITY_REF_NODE)) {
            node = node->children;
        } else if (node->next != NULL) {
            node = node->next;
        } else {
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
    return 0;
}

int main(void) {
    /* Set up a context with a valid output stream */
    struct _xmllintShellCtxt ctxt_s = {0};
    ctxt_s.output = stdout;
    xmllintShellCtxtPtr ctxt = &ctxt_s;

    /* Craft a single comment node with NULL content to trigger the bug */
    xmlNodePtr comment = (xmlNodePtr)calloc(1, sizeof(*comment));
    if (!comment) {
        perror("calloc");
        return 1;
    }
    comment->type = XML_COMMENT_NODE;
    comment->content = NULL; /* This is the crucial condition */
    comment->children = NULL;
    comment->next = NULL;
    comment->parent = NULL;

    char *needle = "match"; /* Non-NULL arg so the function proceeds */

    /* This call will attempt xmlStrstr(comment->content, needle) and crash */
    (void)xmllintShellGrep(ctxt, needle, comment, NULL);

    /* Cleanup (unreached due to crash) */
    free(comment);
    return 0;
}
