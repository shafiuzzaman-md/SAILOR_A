#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal typedefs and structs to model the parser context */
typedef unsigned char xmlChar;

typedef struct _xmlParserInput {
    const xmlChar *cur;
    const xmlChar *end;
    int line;
} xmlParserInput;

typedef struct _xmlSAXHandler {
    void (*setDocumentLocator)(void*, void*);
    void (*startDocument)(void*);
} xmlSAXHandler;

typedef struct _xmlParserCtxt {
    int disableSAX;
    int instate;
    xmlParserInput *input;
    xmlSAXHandler *sax;
    void *userData;
    xmlChar *version;
} xmlParserCtxt;

/* Parser states (only the one we need) */
#define XML_PARSER_XML_DECL 1

/* Whitespace check similar to libxml2's IS_BLANK_CH */
#define IS_BLANK_CH(c) (((c) == ' ') || ((c) == '\t') || ((c) == '\n') || ((c) == '\r'))

/* Stub: xmlParseLookupString - in libxml2 this ensures that a terminator like "?>" exists.
 * For this reproducer we deliberately return true to simulate that the terminator exists
 * somewhere beyond the currently available buffer (streaming case), which is what triggers
 * the buggy peek ahead. */
static int xmlParseLookupString(xmlParserCtxt *ctxt, int start, const char *str, int len) {
    (void)ctxt; (void)start; (void)str; (void)len;
    return 1; /* Pretend we found "?>" */
}

/* Stub: xmlParseXMLDecl - never actually reached before the OOB read happens */
static void xmlParseXMLDecl(xmlParserCtxt *ctxt) {
    (void)ctxt;
}

/* Minimal strdup for xmlChar */
static xmlChar *xmlCharStrdup(const char *s) {
    size_t n = strlen(s) + 1;
    xmlChar *p = (xmlChar*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

/* The vulnerable logic extracted and minimized from parser.c:xmlParseTryOrFinish */
static int xmlParseTryOrFinish(xmlParserCtxt *ctxt, int terminate) {
    int avail;
    int ret = 0;
    int cur, next;

    /* Only model the XML_PARSER_XML_DECL state path */
    avail = (int)(ctxt->input->end - ctxt->input->cur);
    if ((!terminate) && (avail < 2))
        return ret;

    cur = ctxt->input->cur[0];
    next = ctxt->input->cur[1];
    if ((cur == '<') && (next == '?')) {
        /* PI or XML decl */
        if ((!terminate) && (!xmlParseLookupString(ctxt, 2, "?>", 2)))
            return ret;
        /* BUG: The code below peeks at cur[2..5] without ensuring availability >= 6 */
        if ((ctxt->input->cur[2] == 'x') &&
            (ctxt->input->cur[3] == 'm') &&
            (ctxt->input->cur[4] == 'l') &&
            (IS_BLANK_CH(ctxt->input->cur[5]))) {
            ret += 5;
            xmlParseXMLDecl(ctxt);
        } else {
            ctxt->version = xmlCharStrdup("1.0");
        }
    } else {
        ctxt->version = xmlCharStrdup("1.0");
    }
    return ret;
}

int main(void) {
    /* Craft a short buffer that starts with "<?xml" but has no 6th byte available.
     * This forces the code to read input->cur[5] out of bounds when checking for a
     * following blank after "xml". */
    static const xmlChar buf[] = { '<', '?', 'x', 'm', 'l' }; /* length = 5, no extra byte */

    xmlParserInput input;
    input.cur = buf;
    input.end = buf + sizeof(buf); /* avail = 5 bytes */
    input.line = 1;

    xmlParserCtxt ctxt;
    memset(&ctxt, 0, sizeof(ctxt));
    ctxt.disableSAX = 0;
    ctxt.instate = XML_PARSER_XML_DECL;
    ctxt.input = &input;
    ctxt.sax = NULL;
    ctxt.userData = NULL;
    ctxt.version = NULL;

    /* terminate = 0 to follow the non-terminating streaming path */
    (void)xmlParseTryOrFinish(&ctxt, 0);

    /* If ASan didn't abort yet, print something */
    if (ctxt.version) free(ctxt.version);
    puts("Done");
    return 0;
}
