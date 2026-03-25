/* AUTO-GENERATED from harness preamble */
#pragma once

#include <stdlib.h>
#include <stdint.h>

// Minimal typedefs to satisfy signatures
typedef unsigned char xmlChar;
typedef int xmlParserErrors;
typedef int xmlSchemaTypeType;

typedef void* xmlSchemaTreeItemPtr;
typedef void* xmlSchemaParserCtxtPtr;
typedef void* xmlSchemaPtr;
typedef void* xmlSchemaTypePtr;

typedef void* xmlSchemaModelGroupPtr;
typedef void* xmlSchemaParticlePtr;
typedef void* xmlAttrPtr;

// Minimal node with the field used by the vulnerable statement
struct _xmlNode { struct _xmlNode *next; };
typedef struct _xmlNode* xmlNodePtr;

// Vulnerable function — keep signature, minimal locals, and the exact vulnerable line
static xmlSchemaTreeItemPtr
xmlSchemaParseModelGroup(xmlSchemaParserCtxtPtr ctxt, xmlSchemaPtr schema,
                         xmlNodePtr node, xmlSchemaTypeType type,
