/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

typedef unsigned char xmlChar;

typedef enum {
    XML_ELEMENT_NODE = 1,
    XML_TEXT_NODE = 3,
    XML_CDATA_SECTION_NODE = 4
} xmlElementType;

typedef struct _xmlNode {
    struct _xmlNode *last;
    struct _xmlNode *parent;
    struct _xmlNode *children;
    xmlChar *content;
    unsigned short line;
    void *properties;
    int type;
    void *doc;
    void *psvi;
} xmlNode, *xmlNodePtr;

typedef struct _xmlParserInput {
    int line;
} xmlParserInput, *xmlParserInputPtr;

typedef struct _xmlParserCtxt {
    xmlNodePtr node;
    int html;
    int options;
    int nodelen;
    int nodemem;
    xmlParserInputPtr input;
} xmlParserCtxt, *xmlParserCtxtPtr;

