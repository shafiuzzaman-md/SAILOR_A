#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stubs/types to simulate libxml2 environment */
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

typedef struct _xmlAttr {
    xmlChar *name;
    xmlChar *value;
    struct _xmlAttr *next;
} xmlAttr, *xmlAttrPtr;

typedef struct _xmlNode {
    xmlChar *name;            /* element name */
    struct _xmlNode *children;
    struct _xmlNode *parent;
    void *doc;
    void *ns;
    xmlAttrPtr properties;    /* attributes */
} xmlNode, *xmlNodePtr;

/* Helpers */
static xmlChar *xmlStrdupInternal(const xmlChar *s) {
    if (!s) return NULL;
    size_t len = strlen((const char *)s) + 1;
    xmlChar *p = (xmlChar *)malloc(len);
    if (p) memcpy(p, s, len);
    return p;
}

static int xmlStrEqual(const xmlChar *a, const xmlChar *b) {
    if (a == b) return 1;
    if (a == NULL || b == NULL) return 0;
    return strcmp((const char *)a, (const char *)b) == 0;
}

static xmlChar *xmlGetProp(xmlNodePtr node, const xmlChar *name) {
    if (!node || !name) return NULL;
    for (xmlAttrPtr p = node->properties; p; p = p->next) {
        if (xmlStrEqual(p->name, name)) {
            return xmlStrdupInternal(p->value); /* libxml2 returns a copy */
        }
    }
    return NULL;
}

static void xmlUnsetProp(xmlNodePtr node, const xmlChar *name) {
    if (!node || !name) return;
    xmlAttrPtr prev = NULL, cur = node->properties;
    while (cur) {
        if (xmlStrEqual(cur->name, name)) {
            if (prev) prev->next = cur->next; else node->properties = cur->next;
            free(cur->name);
            free(cur->value);
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

static xmlAttrPtr xmlSetProp(xmlNodePtr node, const xmlChar *name, const xmlChar *value) {
    /* Intentionally no NULL check on node to reflect libxml2's expectation */
    /* This will dereference node when node == NULL, reproducing the crash path */
    xmlAttrPtr p = node->properties; /* crash here when node == NULL */
    while (p) {
        if (xmlStrEqual(p->name, name)) {
            free(p->value);
            p->value = xmlStrdupInternal(value);
            return p;
        }
        p = p->next;
    }
    xmlAttrPtr na = (xmlAttrPtr)calloc(1, sizeof(xmlAttr));
    na->name = xmlStrdupInternal(name);
    na->value = xmlStrdupInternal(value);
    na->next = node->properties;
    node->properties = na;
    return na;
}

static void xmlFree(void *ptr) {
    free(ptr);
}

/* The following creation/manipulation stubs mimic failures to hit the bug path */
static xmlNodePtr xmlNewChild(xmlNodePtr parent, void *ns, const xmlChar *name, const xmlChar *content) {
    (void)parent; (void)ns; (void)name; (void)content;
    /* Simulate allocation/creation failure to keep 'text' NULL */
    return NULL;
}

static xmlNodePtr xmlNewDocNode(void *doc, void *ns, const xmlChar *name, const xmlChar *content) {
    (void)doc; (void)ns; (void)name; (void)content;
    /* Not used in this reproducer path (cur->children == NULL), return NULL */
    return NULL;
}

static xmlNodePtr xmlNewDocText(void *doc, const xmlChar *content) {
    (void)doc; (void)content;
    return NULL;
}

static void xmlAddPrevSibling(xmlNodePtr cur, xmlNodePtr elem) {
    (void)cur; (void)elem;
}

static void xmlAddChild(xmlNodePtr parent, xmlNodePtr cur) {
    (void)parent; (void)cur;
}

/* Error reporting stub */
static void xmlRngPErr(void *ctxt, xmlNodePtr cur, int code, const char *msg, const xmlChar *str1, const xmlChar *str2) {
    (void)ctxt; (void)cur; (void)code; (void)str2;
    fprintf(stderr, msg, (const char *)str1);
}

/* Simplified version of the vulnerable function containing the buggy logic */
static void xmlRelaxNGCleanupTree(void *ctxt, xmlNodePtr tree) {
    xmlNodePtr cur = tree;
    if (cur == NULL) return;

    if (xmlStrEqual(cur->name, BAD_CAST "element") || xmlStrEqual(cur->name, BAD_CAST "attribute")) {
        xmlChar *name;
        xmlChar *ns;
        xmlNodePtr text = NULL;

        /* 4.8: name attribute of element and attribute elements */
        name = xmlGetProp(cur, BAD_CAST "name");
        if (name != NULL) {
            if (cur->children == NULL) {
                text = xmlNewChild(cur, cur->ns, BAD_CAST "name", name);
            } else {
                xmlNodePtr node;
                node = xmlNewDocNode(cur->doc, cur->ns, BAD_CAST "name", NULL);
                if (node != NULL) {
                    xmlAddPrevSibling(cur->children, node);
                    text = xmlNewDocText(node->doc, name);
                    xmlAddChild(node, text);
                    text = node;
                }
            }
            if (text == NULL) {
                xmlRngPErr(ctxt, cur, 0, "Failed to create a name %s element\n", name, NULL);
            }
            xmlUnsetProp(cur, BAD_CAST "name");
            xmlFree(name);
            ns = xmlGetProp(cur, BAD_CAST "ns");
            if (ns != NULL) {
                if (text != NULL) {
                    xmlSetProp(text, BAD_CAST "ns", ns);
                }
                xmlFree(ns);
            } else if (xmlStrEqual(cur->name, BAD_CAST "attribute")) {
                /* Vulnerable call: text may be NULL here */
                xmlSetProp(text, BAD_CAST "ns", BAD_CAST "");
            }
        }
    }
}

/* Utility to set an attribute on a node safely during setup */
static void add_setup_prop(xmlNodePtr node, const char *name, const char *value) {
    xmlAttrPtr a = (xmlAttrPtr)calloc(1, sizeof(xmlAttr));
    a->name = xmlStrdupInternal((const xmlChar *)name);
    a->value = xmlStrdupInternal((const xmlChar *)value);
    a->next = node->properties;
    node->properties = a;
}

int main(void) {
    /* Build a minimal node: <attribute name="foo"> with no children and no ns prop */
    xmlNodePtr attr = (xmlNodePtr)calloc(1, sizeof(xmlNode));
    attr->name = xmlStrdupInternal(BAD_CAST "attribute");
    attr->children = NULL; /* ensures xmlNewChild path is taken */
    attr->ns = NULL;
    attr->doc = NULL;
    attr->parent = NULL;
    attr->properties = NULL;

    /* Add name attribute so xmlGetProp("name") succeeds */
    add_setup_prop(attr, "name", "foo");
    /* Intentionally do NOT add an "ns" attribute so ns == NULL */

    /* Call the vulnerable function: this will attempt to create a <name> child,
       fail (text == NULL), then since cur->name == "attribute" and ns == NULL,
       it will call xmlSetProp(text, ... ) with text == NULL and crash. */
    xmlRelaxNGCleanupTree(NULL, attr);

    /* Should not reach here */
    fprintf(stderr, "If you see this, the bug did not trigger as expected.\n");
    return 0;
}
