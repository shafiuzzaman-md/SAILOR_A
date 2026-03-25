#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for libxml2 types/macros used by the vulnerable code */
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

/* Parser context with only the fields that are dereferenced in the bug */
typedef struct _htmlParserCtxt {
    void *_private;
    int instate;
} *htmlParserCtxtPtr;

/* Constants mimicking libxml2 values used in the snippet */
enum { XML_CHAR_ENCODING_UTF8 = 1 };
enum { XML_PARSER_XML_DECL = 0 };
enum { HTML_PARSE_HTML5 = 0x100 };

/* Dummy SAX handler placeholder (type doesn't matter for this reproducer) */
static int tokenizeHtmlSAXHandler;

/* Stub implementations of libxml2 public APIs used by the vulnerable code */
__attribute__((noinline))
htmlParserCtxtPtr htmlCreatePushParserCtxt(void *sax, void *user_data,
                                           const char *chunk, int size,
                                           const char *filename, int enc) {
    (void)sax; (void)user_data; (void)chunk; (void)size; (void)filename; (void)enc;
    /* Simulate allocation failure to make the function return NULL. */
    return NULL;
}

void htmlCtxtUseOptions(htmlParserCtxtPtr ctxt, int options) {
    (void)ctxt; (void)options;
}

void htmlParseChunk(htmlParserCtxtPtr ctxt, const char *data, size_t size, int terminate) {
    (void)ctxt; (void)data; (void)size; (void)terminate;
}

void htmlFreeParserCtxt(htmlParserCtxtPtr ctxt) {
    (void)ctxt;
}

/* Minimal config structure mirroring fields used in the snippet */
typedef struct {
    unsigned int dataState;
    const xmlChar *startTag;
    int inCharacters;
} TokenizerConfig;

/* Vulnerable function skeleton mirroring the key logic from runtest.c */
static int htmlTokenizerTest(const char *filename) {
    (void)filename; /* Unused in this minimal reproducer */

    unsigned int dataState = 0;
    char startTag[31] = "div";
    unsigned int size = 0; /* Unused here, but present in original */

    htmlParserCtxtPtr ctxt;
    TokenizerConfig config;

    /* Create the parser context: in this reproducer it will return NULL */
    ctxt = htmlCreatePushParserCtxt(&tokenizeHtmlSAXHandler, NULL, NULL, 0,
                                    NULL, XML_CHAR_ENCODING_UTF8);

    /* Prepare config as in original code */
    config.dataState = dataState;
    config.startTag = BAD_CAST startTag;
    config.inCharacters = 0;

    /* BUG: Dereference ctxt without checking for NULL */
    /* This write to ctxt->_private will crash due to NULL ctxt */
    ctxt->_private = &config;            /* Null-pointer dereference here */

    /* The rest mirrors the original flow (won't be reached) */
    ctxt->instate = XML_PARSER_XML_DECL; /* Also would dereference NULL */
    htmlCtxtUseOptions(ctxt, HTML_PARSE_HTML5);
    htmlParseChunk(ctxt, NULL, size, 1);
    htmlFreeParserCtxt(ctxt);

    return 0;
}

int main(void) {
    /* Call the vulnerable function, which will immediately crash */
    return htmlTokenizerTest("dummy-input.txt");
}
