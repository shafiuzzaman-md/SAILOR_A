// Self-contained reproducer for NULL dereference in xmlCatalogDumpDoc
// It mimics the relevant parts of libxml2's catalog.c logic shown in the snippet.
// Compile with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdlib.h>

// Minimal stand-ins for libxml2 types
typedef unsigned char xmlChar;

typedef struct _xmlDoc {
    int dummy;
} xmlDoc;

typedef xmlDoc* xmlDocPtr;

typedef struct _xmlCatalog {
    void *xml; // in real libxml2 this would be a catalog entry tree
} xmlCatalog, *xmlCatalogPtr;

// Global state mirroring libxml2's catalog globals
static int xmlCatalogInitialized = 0;
static xmlCatalogPtr xmlDefaultCatalog = NULL;

// Stub for xmlDumpXMLCatalogToDoc; we will not reach it due to the NULL deref
static xmlDocPtr xmlDumpXMLCatalogToDoc(void *unused) {
    (void)unused;
    // Return a non-NULL dummy to show that if we got here things would be fine
    xmlDocPtr doc = (xmlDocPtr)malloc(sizeof(xmlDoc));
    if (doc) doc->dummy = 42;
    return doc;
}

// Simulated xmlInitializeCatalog that fails to create the default catalog but
// still marks the catalog as initialized (as per the bug description)
static void xmlInitializeCatalog(void) {
    // Simulate: xmlCreateNewCatalog() failed -> xmlDefaultCatalog remains NULL
    // But the implementation incorrectly sets xmlCatalogInitialized = 1
    xmlDefaultCatalog = NULL; // explicitly show failure
    xmlCatalogInitialized = 1; // incorrect behavior that triggers the bug later
}

// Vulnerable function copied conceptually from libxml2 (unconditional deref)
static xmlDocPtr xmlCatalogDumpDoc(void) {
    if (!xmlCatalogInitialized)
        xmlInitializeCatalog();

    // NULL dereference here: xmlDefaultCatalog is NULL after failed init
    return xmlDumpXMLCatalogToDoc(xmlDefaultCatalog->xml);
}

int main(void) {
    // Directly call the vulnerable function to trigger the bug path.
    // First call will run xmlInitializeCatalog(), which leaves xmlDefaultCatalog == NULL
    // but sets xmlCatalogInitialized == 1. Then the unconditional dereference crashes.
    (void)xmlCatalogDumpDoc();

    // We should never get here; if we do, free any allocated doc
    // (but in this reproducer, the crash occurs before allocation)
    return 0;
}
