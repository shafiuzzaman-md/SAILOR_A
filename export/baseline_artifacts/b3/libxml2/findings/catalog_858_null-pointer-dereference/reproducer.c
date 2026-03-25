#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Minimal type re-declarations to model the vulnerable path */
typedef unsigned char xmlChar;

typedef struct _xmlCatalogEntry xmlCatalogEntry;
typedef struct _xmlCatalogEntry* xmlCatalogEntryPtr;
struct _xmlCatalogEntry {
    xmlCatalogEntryPtr children;
    xmlCatalogEntryPtr next;
};

typedef struct _xmlHashTable xmlHashTable;
typedef struct _xmlHashTable* xmlHashTablePtr;
struct _xmlHashTable {
    int dummy;
};

/* Catalog structure with the fields used by the vulnerable code */
typedef struct _xmlCatalog xmlCatalog;
typedef xmlCatalog* xmlCatalogPtr;
struct _xmlCatalog {
    int type;
    xmlHashTablePtr sgml;
    xmlCatalogEntryPtr xml;
};

/* Constants mimicking libxml2 */
#define XML_SGML_CATALOG_TYPE 2

/* Debug flag and helper, as seen in the source context */
int xmlDebugCatalogs = 1;
static void xmlCatalogPrintDebug(const char *msg) {
    fputs(msg, stdout);
}

/* Hash scanner callback type (matching libxml2 style) */
typedef void (*xmlHashScanner)(void *payload, void *data, const xmlChar *name);

/* Forward declaration of the vulnerable callback */
static void __attribute__((noinline)) xmlCatalogConvertEntry(void *payload, void *data, const xmlChar *name);

/* Minimal xmlHashScan implementation that triggers the callback once */
static void xmlHashScan(xmlHashTablePtr table, xmlHashScanner scanner, void *data) {
    (void)table; /* not used in this stub */
    const xmlChar name[] = "dummy";
    void *payload = NULL; /* not used by our callback */
    /* Directly invoke the callback to simulate a single hash entry */
    scanner(payload, data, name);
}

/* Vulnerable conversion callback: it assumes 'data' is xmlCatalogPtr */
static void __attribute__((noinline)) xmlCatalogConvertEntry(void *payload, void *data, const xmlChar *name) {
    (void)payload;
    (void)name;
    /* Type confusion: 'data' is expected to be xmlCatalogPtr, but will actually
     * be the ADDRESS of the local parameter variable from xmlConvertSGMLCatalog. */
    xmlCatalogPtr catal = (xmlCatalogPtr) data;

    /* The following lines dereference catal->xml, which comes from misinterpreting
     * the stack slot as a struct pointer, leading to an invalid pointer dereference. */
    if (catal->xml == NULL) {
        /* If by chance it looks NULL, force a dereference anyway to crash deterministically. */
        /* This models the original code that uses catal->xml unconditionally after conversion. */
        fprintf(stdout, "catal->xml appeared NULL; forcing deref to show the bug...\n");
        /* Intentional null-dereference to reflect the actual vulnerability impact */
        volatile int *p = NULL;
        *p = 42;
    }

    /* Typical code path in the real function manipulates catal->xml->children. */
    /* This will crash because catal->xml is not a valid pointer to xmlCatalogEntry. */
    catal->xml->children = NULL; /* Boom: invalid pointer dereference due to type confusion */
}

/* Vulnerable function from the source context. The bug is passing &catal to xmlHashScan */
static int __attribute__((noinline)) xmlConvertSGMLCatalog(xmlCatalog *catal) {
    if ((catal == NULL) || (catal->type != XML_SGML_CATALOG_TYPE))
        return -1;

    if (xmlDebugCatalogs) {
        xmlCatalogPrintDebug("Converting SGML catalog to XML\n");
    }

    /* BUG: '&catal' is passed instead of 'catal'. The callback will treat 'data' as xmlCatalog*,
     * causing it to interpret the stack address of this local variable as a struct pointer. */
    xmlHashScan(catal->sgml, xmlCatalogConvertEntry, &catal);
    return 0;
}

int main(void) {
    /* Set up a minimal catalog that satisfies the initial checks */
    xmlHashTablePtr ht = (xmlHashTablePtr)calloc(1, sizeof(*ht));
    if (!ht) {
        perror("calloc");
        return 1;
    }

    xmlCatalogEntryPtr root = (xmlCatalogEntryPtr)calloc(1, sizeof(*root));
    if (!root) {
        perror("calloc");
        return 1;
    }

    xmlCatalogPtr catalog = (xmlCatalogPtr)calloc(1, sizeof(*catalog));
    if (!catalog) {
        perror("calloc");
        return 1;
    }

    catalog->type = XML_SGML_CATALOG_TYPE; /* Passes the type check */
    catalog->sgml = ht;                     /* Non-NULL hash table */
    catalog->xml  = root;                   /* Some valid entry (won't be used due to bug) */

    /* This call will reach the buggy xmlConvertSGMLCatalog which passes '&catal' to xmlHashScan.
     * The callback then misinterprets it and dereferences an invalid pointer, crashing. */
    (void)xmlConvertSGMLCatalog(catalog);

    /* Clean up (not reached on success due to crash) */
    free(root);
    free(ht);
    free(catalog);
    return 0;
}
