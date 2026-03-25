/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal harness for libxml2 parser vulnerability in xmlParseLookupInternalSubset */
#include <stddef.h>
#include <stdint.h>

/* Minimal type defs to satisfy signatures and fields used */
typedef unsigned char xmlChar;

typedef struct _xmlParserInput {
    const xmlChar *base;
    const xmlChar *cur;
    const xmlChar *end;
    void *buf;  /* unused in harness */
    int flags;  /* unused */
} xmlParserInput, *xmlParserInputPtr;

typedef struct _xmlParserCtxt {
    xmlParserInputPtr input;
    int disableSAX;    /* unused */
    int errNo;         /* unused */
    int instate;       /* unused */
    int options;       /* unused */
    long checkIndex;   /* unused */
    int endCheckState; /* unused */
} xmlParserCtxt, *xmlParserCtxtPtr;

