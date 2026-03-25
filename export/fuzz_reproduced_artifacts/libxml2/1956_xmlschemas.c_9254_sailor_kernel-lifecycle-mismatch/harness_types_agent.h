/* AUTO-GENERATED from harness preamble */
#pragma once

/* harness/spine.c */
#include <stddef.h>
#include <stdlib.h>

/* Minimal local type definitions to avoid pulling project headers */
#ifndef XML_SCHEMA_TYPE_ALL
#define XML_SCHEMA_TYPE_ALL 1
#endif
#ifndef XML_SCHEMA_TYPE_SEQUENCE
#define XML_SCHEMA_TYPE_SEQUENCE 2
#endif
#ifndef XML_SCHEMA_TYPE_CHOICE
#define XML_SCHEMA_TYPE_CHOICE 3
#endif

typedef unsigned char xmlChar;
typedef int xmlParserErrors;

/* Minimal xmlNode with only the field we need */
struct _xmlNode { struct _xmlNode *next; };
typedef struct _xmlNode* xmlNodePtr;

/* Minimal abstract ctxt with only the state field we need */
struct _xmlSchemaAbstractCtxt { int instate; };
typedef struct _xmlSchemaAbstractCtxt* xmlSchemaAbstractCtxtPtr;

typedef void* xmlSchemaTypePtr; /* unused in our slice */
#define ATTRIBUTE_UNUSED

/* Forward decl to satisfy the spine symbol (not used on path) */

/* Entry == Vulnerable function — neutralized to keep only the target case */
int xmlSchemaComplexTypeErr(xmlSchemaAbstractCtxtPtr actxt,
                            xmlParserErrors error,
                            xmlNodePtr node,
                            xmlSchemaTypePtr type ATTRIBUTE_UNUSED,
                            const char *message,
                            int nbval,
                            int nbneg,
                            xmlChar **values) {
    (void)error; (void)type; (void)message; (void)nbval; (void)nbneg; (void)values;

    /* Local setup for the vulnerable statement */
    xmlNodePtr child = node; /* child sourced from caller */
