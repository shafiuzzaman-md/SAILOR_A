#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal type/flag re-declarations to isolate the vulnerable logic */
typedef unsigned char xmlChar;
#define BAD_CAST (const xmlChar *)

/* Match the flag check used by the vulnerable code */
#define XML_INPUT_HAS_ENCODING (1u << 0)

/* Minimal parser input/context structures */
typedef struct _xmlParserInput {
    const xmlChar *cur; /* current read pointer into input buffer */
    unsigned int flags; /* bitfield; must NOT have XML_INPUT_HAS_ENCODING set */
} xmlParserInput;

typedef struct _htmlParserCtxt {
    xmlParserInput *input;
    void *sax;        /* unused in this reproducer */
    void *userData;   /* unused */
    int disableSAX;   /* unused */
    int instate;      /* unused */
} htmlParserCtxt;

/* Stubs matching libxml2 signatures used around the bug site */
static void xmlDetectEncoding(htmlParserCtxt *ctxt) {
    (void)ctxt; /* no-op; keep flags as set by caller */
}

static void xmlSwitchEncoding(htmlParserCtxt *ctxt, int enc) {
    (void)ctxt; (void)enc; /* no-op */
}

/* xmlStrncmp as used by the vulnerable code: compares 'len' bytes unconditionally */
static int xmlStrncmp(const xmlChar *str1, const xmlChar *str2, int len) {
    /* This memcmp will read 'len' bytes from str1 and str2 (or until a diff is found),
       which causes an out-of-bounds read when str1 doesn't have that many bytes available. */
    return memcmp((const void *)str1, (const void *)str2, (size_t)len);
}

/* Reimplementation of the vulnerable part of htmlParseDocument */
int htmlParseDocument(htmlParserCtxt *ctxt) {
    if ((ctxt == NULL) || (ctxt->input == NULL))
        return -1;

    xmlDetectEncoding(ctxt);

    /* Vulnerable check: no size check before comparing 4 bytes at ctxt->input->cur */
    if (((ctxt->input->flags & XML_INPUT_HAS_ENCODING) == 0) &&
        (xmlStrncmp(ctxt->input->cur, BAD_CAST "<?xm", 4) == 0)) {
        xmlSwitchEncoding(ctxt, /* XML_CHAR_ENCODING_UTF8 */ 1);
    }

    return 0;
}

int main(void) {
    /* Truncated input: only 1 byte allocated. */
    unsigned char *buf = (unsigned char *)malloc(1);
    if (!buf)
        return 1;

    /* Make the first byte match '<' so xmlStrncmp needs to read the next byte,
       which is out-of-bounds. This reliably triggers the ASan OOB read. */
    buf[0] = '<';

    xmlParserInput input = {0};
    input.cur = (const xmlChar *)buf; /* Points to a 1-byte buffer */
    input.flags = 0;                  /* Ensure XML_INPUT_HAS_ENCODING is NOT set */

    htmlParserCtxt ctxt = {0};
    ctxt.input = &input;

    /* Triggers the out-of-bounds read inside xmlStrncmp */
    (void)htmlParseDocument(&ctxt);

    free(buf);
    return 0;
}
