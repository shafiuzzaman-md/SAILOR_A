#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal type re-declarations to mimic libxml2 API */
typedef unsigned char xmlChar;
typedef struct _xmlBuffer {
    int dummy;
} xmlBuffer;

typedef struct _xmlEntitiesTable {
    int dummy;
} xmlEntitiesTable;

typedef void* xmlNodePtr;
typedef void* xmlSaveCtxtPtr;

#define XML_ERR_OK 0

/* Stub: Simulate allocation/error by returning NULL */
static xmlSaveCtxtPtr xmlSaveToBuffer(xmlBuffer *buf, const char *encoding, int options) {
    (void)buf; (void)encoding; (void)options;
    return NULL; /* Simulate failure so 'save' is NULL */
}

/* Stub: Intentionally dereference the save context to crash when it's NULL */
static void xmlSaveTree(void *save, void *ent) {
    (void)ent;
    volatile char *p = (volatile char *)save; /* Will be NULL */
    *p = 0x42; /* Null-pointer dereference here */
}

/* Stub: Return success code (won't be reached due to earlier crash) */
static int xmlSaveFinish(void *save) {
    (void)save;
    return XML_ERR_OK;
}

/* Stub: Detach just returns something; not relevant for this crash path */
static void *xmlBufferDetach(xmlBuffer *buf) {
    return (void*)buf;
}

/* Stub: No-op free */
static void xmlFree(void *ptr) {
    (void)ptr;
}

/* Hash scan callback type */
typedef void (*xmlHashScanner)(void *payload, void *data, const xmlChar *name);

/* Stub: Call the scanner once with a fake entity and provided data (save ctxt) */
static void xmlHashScan(xmlEntitiesTable *table, xmlHashScanner scan, void *data) {
    (void)table;
    int fakeEnt = 123;
    const xmlChar name[] = "E";
    scan(&fakeEnt, data, name);
}

/* Callback used by xmlHashScan: reverse arg order and call xmlSaveTree(save, ent) */
static void xmlDumpEntityDeclScan(void *ent, void *save, const xmlChar *name) {
    (void)name;
    xmlSaveTree(save, ent); /* save is NULL, triggers NPD in xmlSaveTree */
}

/* Vulnerable function as in libxml2 (simplified) */
void xmlDumpEntitiesTable(xmlBuffer *buf, xmlEntitiesTable *table) {
    xmlSaveCtxtPtr save;

    if ((buf == NULL) || (table == NULL))
        return;

    save = xmlSaveToBuffer(buf, NULL, 0); /* returns NULL in our stub */
    xmlHashScan(table, xmlDumpEntityDeclScan, save); /* passes NULL to callback */
    if (xmlSaveFinish(save) != XML_ERR_OK)
        xmlFree(xmlBufferDetach(buf));
}

int main(void) {
    /* Prepare minimal inputs to reach the vulnerable path */
    xmlBuffer buf = {0};
    xmlEntitiesTable table = {0};

    /* This call will crash inside xmlSaveTree due to NULL 'save' */
    xmlDumpEntitiesTable(&buf, &table);

    /* Not reached */
    puts("Unexpectedly survived\n");
    return 0;
}
