#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal type re-declarations to mimic libxml2 internals */
typedef unsigned char xmlChar;

typedef struct _xmlParserInput {
    const xmlChar *cur;
    const xmlChar *end;
} xmlParserInput, *xmlParserInputPtr;

struct _xmlSAXHandler {
    void (*cdataBlock)(void *, const xmlChar *, int);
    void (*characters)(void *, const xmlChar *, int);
};

typedef struct _xmlParserCtxt {
    struct _xmlSAXHandler *sax;
    void *userData;
    int disableSAX;
    int options;
    int nameNr;
    int spaceNr;
    int nodeNr;
    xmlParserInputPtr input;
} xmlParserCtxt, *xmlParserCtxtPtr;

/* Macros used by the vulnerable function */
#define GROW do { } while (0)
#define PARSER_STOPPED(ctxt) 0
#define CUR_PTR (ctxt->input->cur)
#define NXT(i)  (*(ctxt->input->cur + (i)))
/* Keep this as 0 to avoid extra buffer accesses in dead branches */
#define CMP9(ptr, c1,c2,c3,c4,c5,c6,c7,c8,c9) 0

/* Stubbed callees (not reached in this reproducer) */
static void xmlParsePI(xmlParserCtxtPtr ctxt)            { (void)ctxt; }
static void xmlParseCDSect(xmlParserCtxtPtr ctxt)        { (void)ctxt; }
static void xmlParseComment(xmlParserCtxtPtr ctxt)       { (void)ctxt; }
static void xmlParseElementEnd(xmlParserCtxtPtr ctxt)    { (void)ctxt; }
static void xmlParseElementStart(xmlParserCtxtPtr ctxt)  { (void)ctxt; }
static void xmlParseReference(xmlParserCtxtPtr ctxt)     { (void)ctxt; }

/* Vulnerable function re-implemented with the problematic check */
static void xmlParseContentInternal(xmlParserCtxtPtr ctxt) {
    int oldNameNr = ctxt->nameNr;
    int oldSpaceNr = ctxt->spaceNr;
    int oldNodeNr = ctxt->nodeNr;
    (void)oldNameNr; (void)oldSpaceNr; (void)oldNodeNr;

    GROW;
    while ((ctxt->input->cur < ctxt->input->end) && (PARSER_STOPPED(ctxt) == 0)) {
        const xmlChar *cur = ctxt->input->cur;

        /* First case : a Processing Instruction. Vulnerable access below. */
        if ((*cur == '<') && (cur[1] == '?')) {
            xmlParsePI(ctxt);
        }
        /* Second case : CDATA section */
        else if (CMP9(CUR_PTR, '<','!','[','C','D','A','T','A','[')) {
            xmlParseCDSect(ctxt);
        }
        /* Third case : comment */
        else if ((*cur == '<') && (NXT(1) == '!') && (NXT(2) == '-') && (NXT(3) == '-')) {
            xmlParseComment(ctxt);
        }
        /* Fourth case : sub-element */
        else if (*cur == '<') {
            if (NXT(1) == '/') {
                if (ctxt->nameNr <= oldNameNr)
                    break;
                xmlParseElementEnd(ctxt);
            } else {
                xmlParseElementStart(ctxt);
            }
        }
        /* Fifth case : reference */
        else if (*cur == '&') {
            xmlParseReference(ctxt);
        } else {
            /* For this reproducer, bail out if nothing matched */
            break;
        }
        /* Prevent infinite loops for the minimal stub */
        break;
    }
}

int main(void) {
    /* Allocate a 1-byte buffer containing only '<'. No following byte is present. */
    xmlChar *buf = (xmlChar *)malloc(1);
    if (!buf) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    buf[0] = '<';

    xmlParserInput input;
    input.cur = buf;           /* points to the single '<' byte */
    input.end = buf + 1;       /* end is exactly one past the single byte */

    xmlParserCtxt ctxt;
    memset(&ctxt, 0, sizeof(ctxt));
    ctxt.input = &input;
    ctxt.nameNr = 0;
    ctxt.spaceNr = 0;
    ctxt.nodeNr = 0;

    /* This call will enter the while loop, see '<' at the last byte,
       and evaluate cur[1] without bounds checking, causing OOB read. */
    xmlParseContentInternal(&ctxt);

    free(buf);
    return 0;
}
