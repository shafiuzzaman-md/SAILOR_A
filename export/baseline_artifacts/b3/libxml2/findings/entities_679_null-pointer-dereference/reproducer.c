#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal type re-declarations to keep this self-contained */
typedef unsigned char xmlChar;

typedef struct _xmlBuffer {
    size_t size;
    xmlChar *content;
} xmlBuffer;

typedef struct _xmlNode {
    int dummy;
} xmlNode, *xmlNodePtr;

typedef struct _xmlEntity {
    int dummy;
} xmlEntity;

/* Opaque save context type */
typedef struct _xmlSaveCtxt xmlSaveCtxt;
typedef xmlSaveCtxt* xmlSaveCtxtPtr;

/* Constants matching libxml2 style */
#define XML_ERR_OK 0

/* Stub implementations to simulate behavior */
static xmlSaveCtxtPtr xmlSaveToBuffer(xmlBuffer *buf, const char *encoding, int options) {
    (void)buf; (void)encoding; (void)options;
    /* Simulate allocation/error failure so xmlSaveToBuffer returns NULL */
    return NULL;
}

static void xmlSaveTree(xmlSaveCtxtPtr save, xmlNodePtr node) {
    (void)node;
    /* Intentionally dereference the save context to expose the NULL deref. */
    volatile char *p = (char *)save;
    /* This will crash when save == NULL, mirroring the real bug site. */
    char c = *p;
    (void)c;
}

static int xmlSaveFinish(xmlSaveCtxtPtr save) {
    (void)save;
    return XML_ERR_OK;
}

static void *xmlBufferDetach(xmlBuffer *buf) {
    /* In real libxml2 this would detach the internal buffer content. */
    void *ret = buf ? buf->content : NULL;
    if (buf) {
        buf->content = NULL;
        buf->size = 0;
    }
    return ret;
}

static void xmlFree(void *ptr) {
    free(ptr);
}

/* Vulnerable function re-implemented from entities.c lines 671-682 */
static void xmlDumpEntityDecl(xmlBuffer *buf, xmlEntity *ent) {
    xmlSaveCtxtPtr save;

    if ((buf == NULL) || (ent == NULL))
        return;

    save = xmlSaveToBuffer(buf, NULL, 0);
    /* BUG: save is not checked for NULL before use */
    xmlSaveTree(save, (xmlNodePtr) ent);              /* NULL deref here */
    if (xmlSaveFinish(save) != XML_ERR_OK)
        xmlFree(xmlBufferDetach(buf));
}

int main(void) {
    /* Set up a dummy buffer and entity to reach the vulnerable code path */
    xmlBuffer *buf = (xmlBuffer *)calloc(1, sizeof(xmlBuffer));
    xmlEntity *ent = (xmlEntity *)calloc(1, sizeof(xmlEntity));

    if (!buf || !ent) {
        fprintf(stderr, "Allocation failed in reproducer setup\n");
        return 1;
    }

    /* Trigger the bug: xmlSaveToBuffer will return NULL (stub),
       and xmlDumpEntityDecl will call xmlSaveTree(save, ...) without a NULL check */
    xmlDumpEntityDecl(buf, ent);

    /* Not reached if the NULL deref is triggered */
    free(ent);
    free(buf);
    return 0;
}
