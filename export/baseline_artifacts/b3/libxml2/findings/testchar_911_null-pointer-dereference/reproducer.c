#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type definitions to avoid depending on libxml2 */
typedef unsigned char xmlChar;

typedef struct _xmlBuffer {
    xmlChar *content;
    int size;
    int use;
} *xmlBufferPtr;

typedef struct _xmlCharEncodingHandler {
    const char *name;
} *xmlCharEncodingHandlerPtr;

/* Stubs mimicking the libxml2 API used by testEncHandler */
static xmlBufferPtr xmlBufferCreate(void) {
    /* Simulate allocation failure to trigger the NULL dereference path */
    return NULL;
}

static int xmlBufferAdd(xmlBufferPtr buf, const xmlChar *content, int len) {
    /* Intentionally no NULL check to mirror the vulnerability surface. */
    /* This will dereference a NULL pointer when buf == NULL. */
    int need = (len >= 0) ? len : (int)strlen((const char *)content);
    (void)need;
    /* Crash here by dereferencing buf */
    int available = buf->size - buf->use; /* NULL dereference */
    (void)available;
    return 0;
}

static const xmlChar *xmlBufferContent(xmlBufferPtr buf) {
    if (!buf || !buf->content) return (const xmlChar *)"";
    return buf->content;
}

static int xmlBufferLength(xmlBufferPtr buf) {
    if (!buf) return 0;
    return buf->use;
}

static void xmlBufferEmpty(xmlBufferPtr buf) {
    if (!buf) return;
    buf->use = 0;
    if (buf->content) buf->content[0] = 0;
}

static void xmlBufferFree(xmlBufferPtr buf) {
    if (!buf) return;
    free(buf->content);
    free(buf);
}

static int xmlCharEncInFunc(xmlCharEncodingHandlerPtr handler, xmlBufferPtr out, xmlBufferPtr in) {
    (void)handler; (void)out; (void)in; return 0;
}

static int xmlCharEncOutFunc(xmlCharEncodingHandlerPtr handler, xmlBufferPtr out, xmlBufferPtr in) {
    (void)handler; (void)out; (void)in; return 0;
}

/* Helper functions copied/adapted from the source context */
static void bufDump(const char *prefix, const xmlChar *content, int len) {
    int i;
    fprintf(stderr, "%s", prefix);
    for (i = 0; i < len; i++) {
        fprintf(stderr, " %02X", content[i]);
    }
    fprintf(stderr, "\n");
}

static int bufCompare(xmlBufferPtr got, const xmlChar *expectContent, int expectLen) {
    const xmlChar *gotContent = xmlBufferContent(got);
    int gotLen = xmlBufferLength(got);

    if ((gotLen == expectLen) && (memcmp(gotContent, expectContent, gotLen) == 0))
        return 0;

    bufDump("got:     ", gotContent, gotLen);
    bufDump("expected:", expectContent, expectLen);

    return -1;
}

/* Vulnerable function: mirrors testchar.c::testEncHandler */
static int testEncHandler(xmlCharEncodingHandlerPtr handler, const xmlChar *dec,
                          int decSize, const xmlChar *enc, int encSize) {
    xmlBufferPtr encBuf = xmlBufferCreate();
    xmlBufferPtr decBuf = xmlBufferCreate();
    int ret = 0;

    /* Vulnerability: encBuf is not checked for NULL before use */
    xmlBufferAdd(encBuf, enc, encSize); /* NULL dereference occurs inside xmlBufferAdd */
    xmlCharEncInFunc(handler, decBuf, encBuf);
    if (bufCompare(decBuf, dec, decSize) != 0) {
        fprintf(stderr, "Decoding %s failed\n", handler->name);
        ret = -1;
    }

#ifdef LIBXML_OUTPUT_ENABLED
    xmlBufferEmpty(decBuf);
    xmlBufferAdd(decBuf, dec, decSize);
    xmlCharEncOutFunc(handler, encBuf, decBuf);
    if (bufCompare(encBuf, enc, encSize) != 0) {
        fprintf(stderr, "Encoding %s failed\n", handler->name);
        ret = -1;
    }
#endif

    xmlBufferFree(decBuf);
    xmlBufferFree(encBuf);
    return ret;
}

int main(void) {
    struct _xmlCharEncodingHandler handler = { "dummy-enc" };

    /* Any small buffers suffice; failure is forced via xmlBufferCreate() */
    static const xmlChar dec[] = { 'A' };
    static const xmlChar enc[] = { 'B' };

    /* This call will trigger the NULL-pointer dereference inside xmlBufferAdd */
    return testEncHandler(&handler, dec, (int)sizeof(dec), enc, (int)sizeof(enc));
}
