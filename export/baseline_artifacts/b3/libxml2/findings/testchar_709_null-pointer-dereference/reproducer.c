#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* Minimal stubs to mirror libxml2 types/symbols used in testUserEncoding */
typedef unsigned char xmlChar;

typedef struct _xmlNode {
    struct _xmlNode *children;
    xmlChar *content;
} xmlNode, *xmlNodePtr;

typedef struct _xmlDoc {
    xmlNodePtr children;
} xmlDoc, *xmlDocPtr;

/* Stubbed libxml2 API (not actually used because we crash earlier) */
xmlDocPtr xmlReadMemory(const char *buffer, int size, const char *URL,
                        const char *encoding, int options) {
    (void)buffer; (void)size; (void)URL; (void)encoding; (void)options;
    return NULL;
}

void xmlFreeDoc(xmlDocPtr doc) {
    (void)doc;
}

void xmlFree(void *ptr) {
    (void)ptr;
}

/* Critical piece: simulate OOM by making xmlMalloc always return NULL. */
void *xmlMalloc(size_t size) {
    (void)size;
    return NULL; /* Force allocation failure */
}

/* Reimplementation of the vulnerable function from testchar.c */
static int testUserEncoding(void) {
    /* Create a document encoded as UTF-16LE with an ISO-8859-1 encoding
     * declaration, then parse it with xmlReadMemory and the encoding
     * argument set to UTF-16LE.
     */
    xmlDocPtr doc = NULL;
    const char *start = "<?xml version='1.0' encoding='ISO-8859-1'?><d>";
    const char *end = "</d>";
    char *buf = NULL;
    xmlChar *text;
    int startSize = (int)strlen(start);
    int textSize = 100000; /* Make sure to exceed internal buffer sizes. */
    int endSize = (int)strlen(end);
    int totalSize = startSize + textSize + endSize;
    int k = 0;
    int i;
    int ret = 1;

    /* Vulnerability: xmlMalloc return value is not checked for NULL */
    buf = (char *)xmlMalloc(2 * (size_t)totalSize);

    /* Immediate write to buf causes NULL pointer dereference on OOM */
    for (i = 0; start[i] != 0; i++) { /* This loop starts at line 709 in testchar.c */
        buf[k++] = start[i];
        buf[k++] = 0;
    }

    for (i = 0; i < textSize; i++) {
        buf[k++] = 'x';
        buf[k++] = 0;
    }
    for (i = 0; end[i] != 0; i++) {
        buf[k++] = end[i];
        buf[k++] = 0;
    }

    /* The rest is unreachable due to the crash above, but kept for fidelity */
    doc = xmlReadMemory(buf, 2 * totalSize, NULL, "UTF-16LE", 0);
    if (doc == NULL) {
        fprintf(stderr, "failed to parse document\n");
        goto error;
    }

    text = doc->children->children->content;
    for (i = 0; i < textSize; i++) {
        if (text[i] != 'x') {
            fprintf(stderr, "text node has wrong content at offset %d\n", k);
            goto error;
        }
    }

    ret = 0;

error:
    xmlFreeDoc(doc);
    xmlFree(buf);

    return ret;
}

int main(void) {
    /* Trigger the vulnerable code path */
    return testUserEncoding();
}
