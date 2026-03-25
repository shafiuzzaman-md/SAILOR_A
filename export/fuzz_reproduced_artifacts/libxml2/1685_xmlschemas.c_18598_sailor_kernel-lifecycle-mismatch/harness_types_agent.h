/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifndef XML_SCHEMA_CTXT_PARSER
#define XML_SCHEMA_CTXT_PARSER 1
#endif

typedef unsigned char xmlChar;
typedef int xmlParserErrors;
typedef struct _xmlSchemaAbstractCtxt { int dummy; } *xmlSchemaAbstractCtxtPtr;
typedef struct _xmlNode { int dummy; } *xmlNodePtr;
typedef struct _xmlSchemaType { int type; } *xmlSchemaTypePtr;

// Vulnerable callee (neutralized to keep only a representative sink)
static int xmlSchemaVAttributesComplex(xmlSchemaAbstractCtxtPtr actxt,
                                       xmlNodePtr node,
                                       xmlSchemaTypePtr type,
                                       int flags,
                                       xmlChar **values,
                                       int nbval) {
    // Representative vulnerable access (stand-in for real statement)
