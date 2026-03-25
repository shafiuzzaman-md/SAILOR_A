#include <stdio.h>
#include <stdlib.h>

/* Minimal stand-ins for libxml2 types/APIs used by example/reader4.c */
typedef unsigned char xmlChar;

typedef struct _xmlDoc {
    const xmlChar *URL;  /* Intentionally left NULL to trigger the bug */
} xmlDoc, *xmlDocPtr;

typedef struct _xmlTextReader {
    xmlDocPtr doc;
    int state; /* 0 -> first read returns 1, then returns -1 (parse error) */
} xmlTextReader, *xmlTextReaderPtr;

/* Stub implementations mimicking the public reader API behavior */
xmlTextReaderPtr xmlReaderForMemory(const char *buffer, int size,
                                    const char *URL, const char *encoding,
                                    int options) {
    (void)buffer; (void)size; (void)URL; (void)encoding; (void)options;
    xmlTextReaderPtr r = (xmlTextReaderPtr)calloc(1, sizeof(xmlTextReader));
    if (!r) return NULL;
    r->doc = (xmlDocPtr)calloc(1, sizeof(xmlDoc));
    if (!r->doc) {
        free(r);
        return NULL;
    }
    /* Critical to reproducer: ensure doc->URL is NULL */
    r->doc->URL = NULL;
    r->state = 0;
    return r;
}

int xmlTextReaderRead(xmlTextReaderPtr readerPtr) {
    if (!readerPtr) return -1;
    /* Simulate reading some nodes successfully once, then a parse error */
    if (readerPtr->state == 0) {
        readerPtr->state = 1;
        return 1; /* keep loop going once */
    }
    return -1; /* ret != 0 triggers the vulnerable fprintf with NULL URL */
}

xmlDocPtr xmlTextReaderCurrentDoc(xmlTextReaderPtr readerPtr) {
    if (!readerPtr) return NULL;
    return readerPtr->doc; /* Non-NULL doc with NULL URL */
}

/* Vulnerable function copied (logically) from example/reader4.c */
static void processDoc(xmlTextReaderPtr readerPtr) {
    int ret;
    xmlDocPtr docPtr;
    const xmlChar *URL;

    ret = xmlTextReaderRead(readerPtr);
    while (ret == 1) {
        ret = xmlTextReaderRead(readerPtr);
    }

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
        /* BUG: URL may be NULL here, leading to NULL dereference in fprintf */
        fprintf(stderr, "%s: Failed to parse\n", URL);
        return;
    }

    printf("%s: Processed ok\n", (const char *)URL);
}

int main(void) {
    /* Create a reader whose document has a NULL URL and will report a parse error */
    const char *bad_xml = "<root><unclosed></root>"; /* Just a placeholder; not actually parsed by stubs */
    xmlTextReaderPtr readerPtr = xmlReaderForMemory(bad_xml, (int)sizeof("<root><unclosed></root>") - 1,
                                                   NULL, NULL, 0);
    if (NULL == readerPtr) {
        fprintf(stderr, "failed to create reader\n");
        return 1;
    }

    processDoc(readerPtr);
    return 0;
}
