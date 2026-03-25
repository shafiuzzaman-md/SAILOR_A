/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal neutralized harness for htmlParseChunk -> htmlParseTryOrFinish */
#include <stddef.h>
#include <stdint.h>

#ifndef XML_PARSER_CONTENT
#define XML_PARSER_CONTENT 4
#endif
#ifndef XML_PARSER_EOF
#define XML_PARSER_EOF 26
#endif

typedef unsigned char xmlChar;

typedef struct _xmlParserInput {
    const xmlChar *base;
    const xmlChar *cur;
    const xmlChar *end;
} xmlParserInput, *xmlParserInputPtr;

typedef struct _htmlParserCtxt {
    xmlParserInputPtr input;
    int instate;
    void *userData;
    void *sax;
    int errNo;
} htmlParserCtxt, *htmlParserCtxtPtr;

/* Forward decl per original source usage */

