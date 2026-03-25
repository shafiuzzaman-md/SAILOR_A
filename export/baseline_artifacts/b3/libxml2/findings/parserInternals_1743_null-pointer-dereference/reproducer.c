#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal type and macro definitions compatible with the vulnerable code */
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

/* Input flags mimicking libxml2's internal flags */
#define XML_INPUT_USES_ENC_DECL (1<<0)
#define XML_INPUT_AUTO_ENCODING (1<<1)
#define XML_INPUT_HAS_ENCODING  (1<<2)

/* Minimal handler/buffer/input/context structures to satisfy field accesses */
typedef struct _xmlCharEncodingHandler {
    const char *name; /* not used for this crash path */
} xmlCharEncodingHandler;

typedef struct _xmlParserInputBuffer {
    xmlCharEncodingHandler *encoder; /* not used for this crash path */
} xmlParserInputBuffer;

typedef struct _xmlParserInput {
    int flags;                      /* used */
    xmlParserInputBuffer *buf;      /* used in other branches */
    const char *filename;           /* unused, here for completeness */
    const char *version;            /* unused */
    void (*free)(xmlChar *);        /* unused */
    const xmlChar *base;            /* unused */
} xmlParserInput;

typedef struct _xmlParserCtxt {
    const xmlChar *encoding;  /* used in first branch */
    xmlParserInput *input;    /* dereferenced without NULL checks */
} xmlParserCtxt;

/* Vulnerable function, reproduced from the source context */
const xmlChar *
xmlGetActualEncoding(xmlParserCtxt *ctxt) {
    const xmlChar *encoding = NULL;

    /* NULL dereference when ctxt is NULL or ctxt->input is NULL */
    if ((ctxt->input->flags & XML_INPUT_USES_ENC_DECL) ||
        (ctxt->input->flags & XML_INPUT_AUTO_ENCODING)) {
        /* Preserve encoding exactly */
        encoding = ctxt->encoding;
    } else if ((ctxt->input->buf) && (ctxt->input->buf->encoder)) {
        encoding = BAD_CAST ctxt->input->buf->encoder->name;
    } else if (ctxt->input->flags & XML_INPUT_HAS_ENCODING) {
        encoding = BAD_CAST "UTF-8";
    }

    return(encoding);
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    /* Case 1: Trigger by passing a NULL parser context. */
    /* This will dereference ctxt->input at the first if-condition and crash. */
    const xmlChar *enc = xmlGetActualEncoding(NULL);

    /* The following lines won't be reached, but keep a use of enc to avoid warnings */
    if (enc)
        printf("Encoding: %s\n", (const char *)enc);

    return 0;
}
