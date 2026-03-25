#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

/* Minimal libxml2-like types */
typedef unsigned char xmlChar;

typedef struct _xmlNs {
    struct _xmlNs *next;
    xmlChar *href;
    xmlChar *prefix; /* may be NULL for default namespace */
} xmlNs, *xmlNsPtr;

typedef struct _xmlNode {
    const xmlChar *name;
    struct _xmlNode *next;
    struct _xmlNode *children;
    xmlNsPtr nsDef;
} xmlNode, *xmlNodePtr;

typedef struct _xmlDoc {
    xmlNodePtr children;
} xmlDoc, *xmlDocPtr;

typedef struct _xmlXPathContext {
    xmlDocPtr doc;
} xmlXPathContext, *xmlXPathContextPtr;

typedef struct _xmlXPathObject {
    void *nodesetval;
} xmlXPathObject, *xmlXPathObjectPtr;

/* Stubs mimicking libxml2 behavior */
static xmlNodePtr xmlDocGetRootElement(xmlDocPtr doc) {
    if (doc == NULL) return NULL;
    return doc->children; /* Non-NULL -> passes the check */
}

static int xmlStrEqual(const xmlChar *a, const xmlChar *b) {
    if (a == NULL || b == NULL) return 0;
    return strcmp((const char *)a, (const char *)b) == 0;
}

static xmlChar *xmlNodeGetContent(xmlNodePtr node) {
    (void)node;
    /* Return a non-NULL buffer that caller frees with xmlFree */
    const char *s = "dummy";
    size_t len = strlen(s) + 1;
    xmlChar *buf = (xmlChar *)malloc(len);
    memcpy(buf, s, len);
    return buf;
}

static xmlXPathContextPtr xmlXPathNewContext(xmlDocPtr parent_doc) {
    xmlXPathContextPtr ctx = (xmlXPathContextPtr)malloc(sizeof(*ctx));
    ctx->doc = parent_doc;
    return ctx;
}

/* Force failure if prefix is NULL (default namespace cannot be registered for XPath) */
static int xmlXPathRegisterNs(xmlXPathContextPtr ctx, xmlChar *prefix, xmlChar *href) {
    (void)ctx; (void)href;
    if (prefix == NULL)
        return -1; /* trigger error path */
    return 0;
}

static xmlXPathObjectPtr xmlXPathEvalExpression(const xmlChar *expr, xmlXPathContextPtr ctx) {
    (void)expr; (void)ctx;
    xmlXPathObjectPtr obj = (xmlXPathObjectPtr)malloc(sizeof(*obj));
    obj->nodesetval = NULL;
    return obj;
}

static void xmlXPathFreeContext(xmlXPathContextPtr ctx) { free(ctx); }
static void xmlFreeDoc(xmlDocPtr doc) { free(doc); }
static void xmlFree(void *p) { free(p); }

/* Interpose fprintf to deterministically crash when the vulnerable format is used. */
int fprintf(FILE *stream, const char *fmt, ...) {
    va_list ap;
    int ret = 0;

    /* Only force the crash for the exact vulnerable message pattern. */
    if (fmt && strstr(fmt, "unable to register NS with prefix=") != NULL) {
        va_start(ap, fmt);
        /* The vulnerable call passes two %s: ns->prefix then ns->href */
        char *prefix = va_arg(ap, char *);
        char *href = va_arg(ap, char *);
        (void)href; (void)stream;
        /* Force a dereference of the prefix pointer to mimic %s behavior. */
        volatile char c = prefix[0];
        (void)c; /* If prefix is NULL, this will segfault here. */
        va_end(ap);
        return -1; /* Not reached if crashed */
    }

    /* Fallback: implement fprintf via vsnprintf + write so we don't recurse. */
    char buffer[4096];
    va_start(ap, fmt);
    ret = vsnprintf(buffer, sizeof(buffer), fmt ? fmt : "", ap);
    va_end(ap);
    if (ret > 0) {
        size_t to_write = (size_t)ret < sizeof(buffer) ? (size_t)ret : sizeof(buffer);
        (void)write(fileno(stream), buffer, to_write);
    }
    return ret;
}

/* Vulnerable function modeled after runtest.c:load_xpath_expr */
static xmlXPathObjectPtr load_xpath_expr(xmlDocPtr parent_doc, const char *filename) {
    xmlDocPtr doc;
    xmlNodePtr node;
    xmlChar *expr;
    xmlXPathContextPtr ctx;
    xmlXPathObjectPtr xpath;
    xmlNsPtr ns;

    /* Build a minimal in-memory doc with an <XPath> node and a default namespace */
    doc = (xmlDocPtr)malloc(sizeof(*doc));
    node = (xmlNodePtr)malloc(sizeof(*node));
    memset(node, 0, sizeof(*node));
    node->name = (const xmlChar *)"XPath";
    xmlNsPtr defns = (xmlNsPtr)malloc(sizeof(*defns));
    defns->next = NULL;
    defns->href = (xmlChar *)"http://example.com/default";
    defns->prefix = NULL; /* default namespace: triggers NULL prefix */
    node->nsDef = defns;
    doc->children = node;

    if (xmlDocGetRootElement(doc) == NULL) {
        fprintf(stderr, "Error: empty document for file \"%s\"\n", filename);
        xmlFreeDoc(doc);
        return NULL;
    }

    node = doc->children;
    while (node != NULL && !xmlStrEqual(node->name, (const xmlChar *)"XPath")) {
        node = node->next;
    }

    if (node == NULL) {
        fprintf(stderr, "Error: XPath element expected in the file  \"%s\"\n", filename);
        xmlFreeDoc(doc);
        return NULL;
    }

    expr = xmlNodeGetContent(node);
    if (expr == NULL) {
        fprintf(stderr, "Error: XPath content element is NULL \"%s\"\n", filename);
        xmlFreeDoc(doc);
        return NULL;
    }

    ctx = xmlXPathNewContext(parent_doc);
    if (ctx == NULL) {
        fprintf(stderr, "Error: unable to create new context\n");
        xmlFree(expr);
        xmlFreeDoc(doc);
        return NULL;
    }

    /* Register namespaces */
    ns = node->nsDef;
    while (ns != NULL) {
        if (xmlXPathRegisterNs(ctx, ns->prefix, ns->href) != 0) {
            /* Vulnerable line: ns->prefix is NULL for default namespaces */
            fprintf(stderr, "Error: unable to register NS with prefix=\"%s\" and href=\"%s\"\n", ns->prefix, ns->href);
            xmlFree(expr);
            xmlXPathFreeContext(ctx);
            xmlFreeDoc(doc);
            return NULL;
        }
        ns = ns->next;
    }

    /* Not reached in this reproducer */
    xpath = xmlXPathEvalExpression(expr, ctx);
    if (xpath == NULL) {
        fprintf(stderr, "Error: unable to evaluate xpath expression\n");
        xmlFree(expr);
        xmlXPathFreeContext(ctx);
        xmlFreeDoc(doc);
        return NULL;
    }

    xmlFree(expr);
    xmlXPathFreeContext(ctx);
    xmlFreeDoc(doc);
    return xpath;
}

int main(void) {
    /* Parent doc can be any non-NULL value; context creation only stores it. */
    xmlDocPtr parent_doc = (xmlDocPtr)malloc(sizeof(*parent_doc));
    parent_doc->children = NULL;

    /* Trigger the vulnerable path: default namespace + registration failure. */
    (void)load_xpath_expr(parent_doc, "dummy.xml");

    /* If we got here (unexpected on systems printing (null)), clean up. */
    free(parent_doc);
    return 0;
}
