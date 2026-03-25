#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal type redefs to emulate libxml2 internals */
typedef unsigned char xmlChar;

typedef struct _xmlParserCtxt {
    int dummy;
} xmlParserCtxt, *xmlParserCtxtPtr;

typedef struct {
    xmlChar *mem;   /* NULL signals the fast path using input buffer */
    size_t size;
} xmlSBuf;

/* Global input buffer to satisfy CUR_PTR usage */
static const xmlChar g_input[] = "abcde";
static size_t g_pos = 3; /* CUR_PTR will point somewhere inside g_input */

#define CUR_PTR ((const xmlChar*)(g_input + g_pos))

/* Stubs for helper functions/macros used in the vulnerable function */
static void xmlSBufAddCString(xmlSBuf *buf, const char *str, int len) {
    (void)buf; (void)str; (void)len;
}

static void xmlSBufAddString(xmlSBuf *buf, const xmlChar *str, int len) {
    (void)buf; (void)str; (void)len;
}

static void xmlSBufCleanup(xmlSBuf *buf, xmlParserCtxtPtr ctxt, const char *msg) {
    (void)buf; (void)ctxt; (void)msg;
}

/* Vulnerable function reimplemented minimally to hit the bug site */
static xmlChar *xmlParseAttValueInternal(xmlParserCtxtPtr ctxt, int normalize,
                                         int *attlen, unsigned int *outFlags) {
    (void)ctxt;
    xmlChar *ret = NULL;
    xmlSBuf buf;
    buf.mem = NULL;   /* Force the (buf.mem == NULL) path */
    buf.size = 0;

    /* Set up state so that we take the buggy branch */
    int inSpace = 1;      /* true */
    int chunkSize = 2;    /* > 0 */
    unsigned int attvalFlags = 0;

    if ((buf.mem == NULL) && (outFlags != NULL)) {
        ret = (xmlChar *) CUR_PTR - chunkSize;

        if (attlen != NULL)
            *attlen = chunkSize;
        /* BUG: Missing attlen NULL-check before decrement */
        if ((normalize) && (inSpace) && (chunkSize > 0)) {
            attvalFlags |= 1; /* XML_ATTVAL_NORM_CHANGE (dummy) */
            *attlen -= 1;     /* NULL deref when caller passed attlen == NULL */
        }

        /* Report potential error */
        xmlSBufCleanup(&buf, ctxt, "AttValue length too long");
    } else {
        /* Not taken in this reproducer */
    }

    if (outFlags != NULL)
        *outFlags = attvalFlags;

    return ret;
}

int main(void) {
    xmlParserCtxt ctxt;
    memset(&ctxt, 0, sizeof(ctxt));

    unsigned int outFlags = 0;

    /* Pass attlen == NULL and outFlags != NULL to trigger the bug */
    (void)xmlParseAttValueInternal(&ctxt, /*normalize=*/1,
                                   /*attlen=*/NULL,
                                   /*outFlags=*/&outFlags);

    /* We should never reach here due to the NULL pointer dereference */
    printf("If you see this, the bug did not trigger as expected.\n");
    return 0;
}
