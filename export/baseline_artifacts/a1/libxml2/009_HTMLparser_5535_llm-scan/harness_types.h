/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c - minimal sliced harness for htmlCtxtSetOptions -> htmlCtxtSetOptionsInternal */
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type universe */
typedef unsigned char xmlChar;

#ifndef HTML_PARSE_RECOVER
#define HTML_PARSE_RECOVER     (1u<<0)
#endif
#ifndef HTML_PARSE_HTML5
#define HTML_PARSE_HTML5       (1u<<1)
#endif
#ifndef HTML_PARSE_NODEFDTD
#define HTML_PARSE_NODEFDTD    (1u<<2)
#endif
#ifndef HTML_PARSE_NOERROR
#define HTML_PARSE_NOERROR     (1u<<3)
#endif
#ifndef HTML_PARSE_NOWARNING
#define HTML_PARSE_NOWARNING   (1u<<4)
#endif
#ifndef HTML_PARSE_PEDANTIC
#define HTML_PARSE_PEDANTIC    (1u<<5)
#endif
#ifndef HTML_PARSE_NOBLANKS
#define HTML_PARSE_NOBLANKS    (1u<<6)
#endif
#ifndef HTML_PARSE_NONET
#define HTML_PARSE_NONET       (1u<<7)
#endif
#ifndef HTML_PARSE_NOIMPLIED
#define HTML_PARSE_NOIMPLIED   (1u<<8)
#endif
#ifndef HTML_PARSE_COMPACT
#define HTML_PARSE_COMPACT     (1u<<9)
#endif
#ifndef HTML_PARSE_HUGE
#define HTML_PARSE_HUGE        (1u<<10)
#endif
#ifndef HTML_PARSE_IGNORE_ENC
#define HTML_PARSE_IGNORE_ENC  (1u<<11)
#endif
#ifndef HTML_PARSE_BIG_LINES
#define HTML_PARSE_BIG_LINES   (1u<<12)
#endif
#ifndef XML_PARSE_NOENT
#define XML_PARSE_NOENT        (1u<<13)
#endif

/* Forward/opaque types */
typedef struct _xmlDict xmlDict;

struct _xmlSAXHandler {
    void (*ignorableWhitespace)(void *ctx, const xmlChar *ch, int len);
};
typedef struct _xmlSAXHandler xmlSAXHandler;

struct _xmlParserCtxt {
    struct _xmlSAXHandler *sax;
    void *userData;
    void *myDoc;
    int wellFormed;
    int options;
    int keepBlanks;
    int recovery;
    xmlDict *dict;
    int dictNames;
};
typedef struct _xmlParserCtxt xmlParserCtxt;
typedef xmlParserCtxt *xmlParserCtxtPtr;
typedef xmlParserCtxt htmlParserCtxt; /* htmlParserCtxt is an alias here */

/* Externals referenced in vulnerable code (definitions provided by stubs or auto-stubs) */

