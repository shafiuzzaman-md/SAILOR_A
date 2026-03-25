/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for libxml2 HTMLparser.c target */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type defs to satisfy signatures */
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

typedef struct _xmlParserInput xmlParserInput;
typedef xmlParserInput *xmlParserInputPtr;
struct _xmlParserInput {
    void *buf;            /* unused here */
    const char *filename; /* unused */
    const char *directory;/* unused */
    const xmlChar *base;
    const xmlChar *cur;
    const xmlChar *end;
    int length;           /* unused */
    int line;
    int col;
};

typedef struct _htmlParserCtxt htmlParserCtxt;
typedef htmlParserCtxt *htmlParserCtxtPtr;
struct _htmlParserCtxt {
    xmlParserInputPtr input;
    int nameNr;           /* unused in harness path */
    int endCheckState;    /* unused in harness path */
    void *sax;            /* unused */
    void *userData;       /* unused */
};

