/* AUTO-GENERATED from harness preamble */
#pragma once

/* Minimal sliced harness for libxml2 xmlschemas.c target
 * Entry: xmlSchemaComplexTypeErr -> Vulnerable: xmlSchemaParseModelGroup
 */
#include <stdlib.h>

#ifndef LIBXML_SCHEMAS_ENABLED
#define LIBXML_SCHEMAS_ENABLED 1
#endif

/* Minimal type universe to satisfy signatures */
typedef unsigned char xmlChar;
typedef struct _xmlNode { void *dummy; } *xmlNodePtr;
typedef struct _xmlAttr { void *dummy; } *xmlAttrPtr;
typedef struct _xmlSchema { int dummy; } *xmlSchemaPtr;

typedef int xmlSchemaTypeType;           /* compositor/type enum */
typedef void* xmlSchemaTreeItemPtr;      /* opaque return type */

typedef struct _xmlSchemaRedef {
    const xmlChar *refTargetNs;
    const xmlChar *refName;
    void *reference;
} xmlSchemaRedef, *xmlSchemaRedefPtr;

typedef struct _xmlSchemaParserCtxt {
    xmlSchemaRedefPtr redef;   /* <- stale pointer target */
    int redefCounter;
} xmlSchemaParserCtxt, *xmlSchemaParserCtxtPtr;

/* Opaque forward decls kept as void* to preserve local declarations */
typedef void* xmlSchemaModelGroupPtr;
typedef void* xmlSchemaParticlePtr;

/* Externals (to be stubbed) */

/* === ENTRY FUNCTION (neutralized pass-through) === */
void xmlSchemaComplexTypeErr(void *actxt,
                             int error,
                             xmlNodePtr node,
                             void *type /* ATTRIBUTE_UNUSED */,
                             const char *message,
                             int nbval,
                             int nbneg,
                             xmlChar **values)
{
    /* MANDATORY: direct call with matching parameters, no guards */
    xmlSchemaParseModelGroup((xmlSchemaParserCtxtPtr)actxt,
                             (xmlSchemaPtr)0,
                             node,
                             (xmlSchemaTypeType)0,
                             0);
    return;
}

/* === VULNERABLE FUNCTION (neutralized to target case) === */
static xmlSchemaTreeItemPtr
xmlSchemaParseModelGroup(xmlSchemaParserCtxtPtr ctxt, xmlSchemaPtr schema,
                         xmlNodePtr node, xmlSchemaTypeType type,
