/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for xmlParseEntityValue -> xmlExpandPEsInEntityValue */
#include <stdint.h>
#include <stddef.h>

/* Minimal types to satisfy signatures */
typedef unsigned char xmlChar;
typedef struct _xmlParserInput {
    const xmlChar *base;
    const xmlChar *cur;
    const xmlChar *end;
    void *buf;
} xmlParserInput;

typedef struct _xmlParserCtxt {
    int options;
    xmlParserInput *input;
    int inputNr;
    int instate;
} xmlParserCtxt;

typedef xmlParserCtxt* xmlParserCtxtPtr;

typedef struct _xmlSBuf { int dummy; } xmlSBuf;

#ifndef PARSER_STOPPED
#define PARSER_STOPPED(ctxt) (0)
#endif

/* VULNERABLE function: keep signature and vulnerable path only */
