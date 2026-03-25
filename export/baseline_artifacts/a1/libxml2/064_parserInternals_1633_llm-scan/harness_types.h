/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stddef.h>
#include <stdint.h>

// Minimal local typedefs to satisfy signature and field accesses
typedef unsigned char xmlChar;

typedef struct _xmlParserInput {
    int flags;
    unsigned char *cur;
} xmlParserInput;

typedef struct _xmlParserCtxt {
    xmlParserInput *input;
    int options;
    int html;
    void *convImpl;
    void *convCtxt;
} xmlParserCtxt;

#ifndef XML_INPUT_HAS_ENCODING
#define XML_INPUT_HAS_ENCODING 0x1
#endif
#ifndef XML_PARSE_IGNORE_ENC
#define XML_PARSE_IGNORE_ENC 0x2
#endif

// Vulnerable function (entry == vulnerable). Keep the exact vulnerable statement.
