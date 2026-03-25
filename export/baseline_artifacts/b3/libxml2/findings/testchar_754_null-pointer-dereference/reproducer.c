#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal stand-ins for libxml2 types/APIs used by testchar.c:convert */
typedef unsigned char xmlChar;
typedef void* xmlCharEncodingHandlerPtr;

typedef struct _xmlBuffer {
    char *content;
    size_t use;
    size_t size;
} xmlBuffer, *xmlBufferPtr;

#define BAD_CAST (xmlChar *)

/* Stub that simulates allocation failure to trigger the bug */
xmlBufferPtr xmlBufferCreate(void) {
    return NULL; /* simulate OOM so 'in' becomes NULL in convert() */
}

/* Stub that will dereference the buffer pointer (matches the expectation
   that the caller ensures non-NULL). Since xmlBufferCreate returned NULL,
   this will crash with a NULL pointer dereference. */
int xmlBufferAdd(xmlBufferPtr buf, const xmlChar *str, int len) {
    if (len < 0) len = (int)strlen((const char*)str);
    /* Force a dereference of buf to reproduce the NPD */
    buf->use += (size_t)len;  /* buf is NULL => ASan-reported NPD */
    return 0;
}

/* No-op stubs for the rest of the API used by convert() */
void xmlCharEncOutFunc(xmlCharEncodingHandlerPtr handler, xmlBufferPtr out, xmlBufferPtr in) {
    (void)handler; (void)out; (void)in;
}

char *xmlBufferDetach(xmlBufferPtr buf) {
    if (!buf) return NULL;
    char *ret = buf->content;
    buf->content = NULL;
    buf->use = 0;
    buf->size = 0;
    return ret;
}

void xmlBufferFree(xmlBufferPtr buf) {
    if (!buf) return;
    free(buf->content);
    free(buf);
}

/* Vulnerable function from testchar.c (logic preserved) */
static char *
convert(xmlCharEncodingHandlerPtr handler, const char *utf8, int size,
        int *outSize) {
    xmlBufferPtr in, out;
    char *ret;

    in = xmlBufferCreate();
    /* Bug: 'in' is not checked for NULL before use */
    xmlBufferAdd(in, BAD_CAST utf8, size);
    out = xmlBufferCreate();
    xmlCharEncOutFunc(handler, out, in);

    if (outSize)
        *outSize = out ? (int)out->use : 0;
    ret = (char *) xmlBufferDetach(out);

    xmlBufferFree(out);
    xmlBufferFree(in);
    return ret;
}

int main(void) {
    const char *utf8 = "trigger";
    int outSize = 0;

    /* This call will hit xmlBufferAdd with a NULL 'in' and crash */
    char *res = convert(NULL, utf8, (int)strlen(utf8), &outSize);

    /* Should not reach here; clean up just in case */
    free(res);
    printf("Unexpectedly survived\n");
    return 0;
}
