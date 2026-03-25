#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* Minimal stand-ins for libxml2 types used by example/reader4.c */
typedef unsigned char xmlChar;

typedef struct _xmlDoc {
    const xmlChar *URL; /* Will be left NULL to trigger the bug */
} xmlDoc, *xmlDocPtr;

typedef struct _xmlTextReader {
    int read_call_count;
    xmlDocPtr doc;
} xmlTextReader, *xmlTextReaderPtr;

/* Stubbed reader API */
int xmlTextReaderRead(xmlTextReaderPtr reader) {
    if (!reader) return -1;
    /* Return 0 immediately to simulate end-of-stream */
    if (reader->read_call_count == 0) {
        reader->read_call_count++;
        return 0;
    }
    return 0;
}

xmlDocPtr xmlTextReaderCurrentDoc(xmlTextReaderPtr reader) {
    if (!reader) return NULL;
    return reader->doc;
}

xmlTextReaderPtr xmlReaderForFile(const char *filename, const char *enc, int options) {
    (void)filename; (void)enc; (void)options;
    xmlTextReaderPtr r = (xmlTextReaderPtr)calloc(1, sizeof(xmlTextReader));
    if (!r) return NULL;
    r->doc = (xmlDocPtr)calloc(1, sizeof(xmlDoc));
    if (!r->doc) { free(r); return NULL; }
    /* Intentionally leave r->doc->URL = NULL to trigger the bug */
    r->read_call_count = 0;
    return r;
}

void xmlFreeReader(xmlTextReaderPtr r) {
    if (!r) return;
    free(r->doc);
    free(r);
}

/* Make the NULL %s dereference deterministic across libcs that would print "(null)" */
#define printf crashy_printf
static int crashy_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    /* The only printf call we make is with format "%s: ..." so the first vararg is a char* */
    char *s = va_arg(ap, char*);
    /* Force a read from s to guarantee a NULL pointer dereference if s == NULL */
    volatile char ch = s[0];
    (void)ch;
    va_end(ap);
    return 0;
}

/* Vulnerable function taken from example/reader4.c (logic preserved) */
static void processDoc(xmlTextReaderPtr readerPtr) {
    int ret;
    xmlDocPtr docPtr;
    const xmlChar *URL;

    ret = xmlTextReaderRead(readerPtr);
    while (ret == 1) {
        ret = xmlTextReaderRead(readerPtr);
    }

    /* Obtain the document */
    docPtr = xmlTextReaderCurrentDoc(readerPtr);
    if (NULL == docPtr) {
        fprintf(stderr, "failed to obtain document\n");
        return;
    }

    URL = docPtr->URL;
    if (NULL == URL) {
        fprintf(stderr, "Failed to obtain URL\n");
    }

    if (ret != 0) {
        fprintf(stderr, "%s: Failed to parse\n", URL);
        return;
    }

    /* BUG: URL may be NULL, but it's passed to printf with "%s" */
    printf("%s: Processed ok\n", (const char *)URL);
}

int main(void) {
    xmlTextReaderPtr readerPtr = xmlReaderForFile("dummy.xml", NULL, 0);
    if (NULL == readerPtr) {
        fprintf(stderr, "failed to create reader\n");
        return 1;
    }

    /* Trigger the vulnerable path: URL is NULL in the stubbed doc */
    processDoc(readerPtr);

    xmlFreeReader(readerPtr);
    return 0;
}
