/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

// Minimal types to support the harness
typedef unsigned char xmlChar;

typedef struct _xmlParserInput {
    const xmlChar *cur;
    const xmlChar *end;
    int col;
} xmlParserInput, *xmlParserInputPtr;

typedef struct _xmlParserCtxt {
    xmlParserInputPtr input;
    void *dict;
    int options;
} xmlParserCtxt, *xmlParserCtxtPtr;

typedef struct {
    const xmlChar *name;
    unsigned long hashValue;
} xmlHashedString;

// Neutralized vulnerable function keeping the vulnerable statement verbatim
