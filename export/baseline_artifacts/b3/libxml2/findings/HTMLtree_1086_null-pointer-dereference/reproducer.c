#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for libxml2 types used by HTMLtree.c */
typedef unsigned char xmlChar;

typedef struct _xmlNs {
    const xmlChar *prefix;
} xmlNs, *xmlNsPtr;

/* Node types (only the ones we need) */
#define HTML_TEXT_NODE 3
#define HTML_ENTITY_REF_NODE 5
#define XML_HTML_DOCUMENT_NODE 13
#define XML_DOCUMENT_NODE 9

typedef struct _xmlNode xmlNode;
typedef struct _xmlNode *xmlNodePtr;

struct _xmlNode {
    void * _private;
    xmlNodePtr next;
    xmlNodePtr parent;
    xmlNodePtr children;
    xmlNodePtr last;
    int type;
    xmlNsPtr ns;
    const xmlChar *name;
    xmlChar *content;
};

/* html element description */
typedef struct _htmlElemDesc {
    int isinline;
} htmlElemDesc;

typedef void* xmlOutputBufferPtr; /* Opaque in real libxml2; not used here */

/* Stubs for I/O used by the serializer */
int xmlOutputBufferWrite(xmlOutputBufferPtr buf, int len, const char *str) {
    (void)buf; (void)len; (void)str;
    return len;
}
int xmlOutputBufferWriteString(xmlOutputBufferPtr buf, const char *str) {
    (void)buf; (void)str;
    return (str != NULL) ? (int)strlen(str) : 0;
}

/* Minimal lookup that returns a non-inline element for "div" */
const htmlElemDesc* htmlTagLookup(const xmlChar *tag) {
    static const htmlElemDesc block_desc = { .isinline = 0 };
    static const htmlElemDesc inline_desc = { .isinline = 1 };
    if (tag && strcmp((const char*)tag, "div") == 0)
        return &block_desc;     /* known non-inline */
    return &inline_desc;        /* default inline */
}

/* Vulnerable function re-declaration with the faulty condition from HTMLtree.c */
void htmlNodeDumpInternal(xmlOutputBufferPtr buf, void *doc, xmlNodePtr cur, int level, int format) {
    (void)buf; (void)doc; (void)level;

    const htmlElemDesc *info;
    xmlNodePtr metaHead = NULL; /* not needed for the crash */

    /* This mirrors the vulnerable branch in HTMLtree.c around line 1080+ */
    if ((format) && (cur->ns == NULL))
        info = htmlTagLookup(cur->name);
    else
        info = NULL;

    /*
     * The following condition dereferences cur->last without checking for NULL.
     * If cur->last == NULL, the evaluation of (cur->last->type ...) will
     * crash with a NULL pointer dereference.
     */
    if ((format) && (info != NULL) && (!info->isinline) &&
        (cur->last->type != HTML_TEXT_NODE) &&
        (cur->last->type != HTML_ENTITY_REF_NODE) &&
        ((cur->children != cur->last) || (cur == metaHead)) &&
        (cur->name != NULL) &&
        (cur->name[0] != 'p')) /* p, pre, param */
    {
        xmlOutputBufferWrite(buf, 1, "\n");
    }

    /* Normally more serialization would follow... */
}

int main(void) {
    /* Craft a node representing a block-level element with no children. */
    xmlNode div;
    memset(&div, 0, sizeof(div));

    div.type = 1;                  /* XML_ELEMENT_NODE (value not critical here) */
    div.name = (const xmlChar*)"div"; /* Non-inline known element */
    div.ns = NULL;                 /* Ensure formatting/tag lookup path is taken */
    div.children = NULL;           /* No children */
    div.last = NULL;               /* CRUCIAL: triggers NULL deref in condition */
    div.parent = NULL;
    div.next = NULL;
    div.content = NULL;

    /* format = 1 to satisfy the first condition; buf/doc are unused in our stub */
    htmlNodeDumpInternal(NULL, NULL, &div, 0, 1);

    /* If the bug didn't crash (it should), exit normally. */
    return 0;
}
