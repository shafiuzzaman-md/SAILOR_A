#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal type re-declarations to mimic libxml2's catalog parser context */
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

static inline int xmlStrEqual(const xmlChar *a, const xmlChar *b) {
    if (a == NULL || b == NULL)
        return 0;
    return strcmp((const char *)a, (const char *)b) == 0;
}

/* Minimal xmlNode representation */
typedef struct _xmlNode {
    xmlChar *name;
    struct _xmlNode *children;
    struct _xmlNode *next;
} xmlNode, *xmlNodePtr;

/* Catalog entry structures and constants */
#define XML_CATA_NEXT_CATALOG 0xCA7A10  /* Arbitrary constant, must match between creator and checker */

typedef struct _xmlCatalogEntry {
    int type;
    xmlChar *URL;
    int prefer;
    int group;
    struct _xmlCatalogEntry *parent;
    struct _xmlCatalogEntry *children;
    struct _xmlCatalogEntry *next;
} xmlCatalogEntry, *xmlCatalogEntryPtr;

/* Debug stubs used by the real code */
int xmlDebugCatalogs = 0;
static void xmlCatalogPrintDebug(const char *fmt, const xmlChar *str) {
    (void)fmt; (void)str; /* no-op */
}

static void xmlFreeCatalogEntry(xmlCatalogEntryPtr entry, void *unused) {
    (void)unused;
    if (!entry) return;
    free(entry->URL);
    free(entry);
}

static void xmlFree(void *p) { free(p); }

/* Stub for xmlParseXMLCatalogOneNode that simulates failure (returns NULL)
 * This matches the scenario where the 'catalog' attribute is missing or URI construction fails.
 */
static xmlCatalogEntryPtr xmlParseXMLCatalogOneNode(xmlNodePtr cur,
                                                   int type,
                                                   const xmlChar *name,
                                                   const xmlChar *idAttr,
                                                   const xmlChar *uriAttr,
                                                   int prefer,
                                                   xmlCatalogEntryPtr cgroup) {
    (void)cur; (void)type; (void)name; (void)idAttr; (void)uriAttr; (void)prefer; (void)cgroup;
    /* Simulate error path: missing required attribute -> returns NULL */
    return NULL;
}

/* Reimplementation of the vulnerable portion of xmlParseXMLCatalogNode */
static void xmlParseXMLCatalogNode(xmlNodePtr cur, int prefer,
                                   xmlCatalogEntryPtr parent, xmlCatalogEntryPtr cgroup) {
    xmlCatalogEntryPtr entry = NULL;
    xmlChar *base = NULL;

    if (cur && xmlStrEqual(cur->name, BAD_CAST"nextCatalog")) {
        xmlCatalogEntryPtr prev = parent ? parent->children : NULL;

        entry = xmlParseXMLCatalogOneNode(cur, XML_CATA_NEXT_CATALOG,
                                          BAD_CAST"nextCatalog", NULL,
                                          BAD_CAST"catalog", prefer, cgroup);
        /* Avoid duplication of nextCatalog */
        while (prev != NULL) {
            if ((prev->type == XML_CATA_NEXT_CATALOG) &&
                (xmlStrEqual(prev->URL, entry->URL)) &&  /* entry is NULL -> NULL deref here */
                (prev->prefer == entry->prefer) &&
                (prev->group == entry->group)) {
                if (xmlDebugCatalogs)
                    xmlCatalogPrintDebug("Ignoring repeated nextCatalog %s\n", entry->URL);
                xmlFreeCatalogEntry(entry, NULL);
                entry = NULL;
                break;
            }
            prev = prev->next;
        }
    }
    if (entry != NULL) {
        if (parent != NULL) {
            entry->parent = parent;
            if (parent->children == NULL)
                parent->children = entry;
            else {
                xmlCatalogEntryPtr p = parent->children;
                while (p->next != NULL)
                    p = p->next;
                p->next = entry;
            }
        }
    }
    if (base != NULL)
        xmlFree(base);
}

int main(void) {
    /* Set up a parent catalog entry with an existing child, so prev != NULL */
    xmlCatalogEntryPtr parent = (xmlCatalogEntryPtr)calloc(1, sizeof(xmlCatalogEntry));
    if (!parent) return 1;

    xmlCatalogEntryPtr existing = (xmlCatalogEntryPtr)calloc(1, sizeof(xmlCatalogEntry));
    if (!existing) return 1;

    existing->type = XML_CATA_NEXT_CATALOG;  /* Ensure first condition is true */
    existing->URL = (xmlChar *)strdup("existing-catalog.xml");
    existing->prefer = 1;
    existing->group = 42;

    parent->children = existing;

    /* Create a node named nextCatalog that will cause xmlParseXMLCatalogOneNode to return NULL */
    xmlNodePtr cur = (xmlNodePtr)calloc(1, sizeof(xmlNode));
    if (!cur) return 1;
    cur->name = BAD_CAST"nextCatalog";

    /* Call into the vulnerable code path: this will dereference entry (NULL) in the de-dup loop */
    xmlParseXMLCatalogNode(cur, /*prefer*/1, parent, /*cgroup*/NULL);

    /* Cleanup (unreached if ASan aborts on NULL deref) */
    free(cur);
    xmlFreeCatalogEntry(existing, NULL);
    xmlFreeCatalogEntry(parent, NULL);

    return 0;
}
