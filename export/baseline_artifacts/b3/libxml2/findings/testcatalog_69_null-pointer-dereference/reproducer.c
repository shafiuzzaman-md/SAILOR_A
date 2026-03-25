#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal libxml2-like typedefs and API stubs to reproduce the bug path */
typedef unsigned char xmlChar;

typedef struct _xmlNode xmlNode;
typedef struct _xmlDoc xmlDoc;

typedef xmlNode* xmlNodePtr;
typedef xmlDoc*  xmlDocPtr;

struct _xmlNode {
    xmlNodePtr children;
    xmlNodePtr next;
};

struct _xmlDoc {
    int dummy; /* no root element stored here to simulate an empty document */
};

#define BAD_CAST (const xmlChar *)

/* Public API stubs */
void xmlInitParser(void) {
    /* no-op */
}

int xmlLoadCatalog(const char *filename) {
    (void)filename;
    /* Pretend loading succeeded */
    return 0;
}

const xmlChar *xmlCatalogResolveURI(const xmlChar *URI) {
    (void)URI;
    /* Simulate resolution attempt but ensure it returns NULL to mimic failure */
    return NULL;
}

xmlDocPtr xmlCatalogDumpDoc(void) {
    /* Return a non-NULL doc that has NO root element, mimicking an empty dump */
    xmlDocPtr doc = (xmlDocPtr)malloc(sizeof(*doc));
    if (doc)
        doc->dummy = 0;
    return doc;
}

void xmlCatalogCleanup(void) {
    /* no-op */
}

xmlNodePtr xmlDocGetRootElement(xmlDocPtr doc) {
    (void)doc;
    /* Critical behavior to trigger the bug: return NULL to indicate no root */
    return NULL;
}

void xmlFreeDoc(xmlDocPtr doc) {
    free(doc);
}

/* Reimplementation of the vulnerable test function */
static int testRepeatedNextCatalog(void) {
    int ret = 0;
    int i = 0;
    const char *cat = "test/catalogs/repeated-next-catalog.xml";
    const char *entity = "/foo.ent";
    xmlDocPtr doc = NULL;
    xmlNodePtr node = NULL;

    xmlInitParser();

    xmlLoadCatalog(cat);
    /* To force the complete recursive load */
    xmlCatalogResolveURI(BAD_CAST entity);
    /**
     * Ensure that the doc doesn't contain the same nextCatalog
     */
    doc = xmlCatalogDumpDoc();
    xmlCatalogCleanup();

    if (doc == NULL) {
        fprintf(stderr, "CATALOG-FAILURE: Failed to dump the catalog\n");
        return 1;
    }

    /* Just the root "catalog" node with a series of nextCatalog */
    node = xmlDocGetRootElement(doc);
    /* BUG: node may be NULL. The next line dereferences it without a check. */
    node = node->children; /* Intentional null-pointer dereference */

    for (i = 0; node != NULL; node = node->next, i++) {}
    if (i > 1) {
        fprintf(stderr, "CATALOG-FAILURE: Found %d nextCatalog entries and should be 1\n", i);
        ret = 1;
    }

    xmlFreeDoc(doc);

    return ret;
}

int main(void) {
    /* Directly invoke the vulnerable function to trigger the crash */
    return testRepeatedNextCatalog();
}
