#include <stdio.h>
#include <stdlib.h>

/* Minimal typedefs/macros to mirror the original code context */
typedef unsigned char xmlChar;
#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

/* Globals as in runtest.c */
static int callbacks = 0;
static int quiet = 0;
static FILE *SAXdebug = NULL;

/* Vulnerable function copied/adapted from the source context */
static void
notationDeclDebug(void *ctx ATTRIBUTE_UNUSED, const xmlChar *name,
                  const xmlChar *publicId, const xmlChar *systemId)
{
    callbacks++;
    if (quiet)
        return;
    /* BUG: publicId and systemId are used with %s without NULL checks */
    fprintf(SAXdebug, "SAX.notationDecl(%s, %s, %s)\n",
            (char *) name, (char *) publicId, (char *) systemId);
}

int main(void) {
    /* Direct output to stdout like the original debug stream */
    SAXdebug = stdout;

    /* Craft inputs to simulate a notation declaration where one identifier is absent.
     * According to XML, either publicId or systemId may be missing (NULL). */
    const xmlChar *name = (const xmlChar *)"MyNotation";
    const xmlChar *publicId = NULL;              /* Absent publicId */
    const xmlChar *systemId = (const xmlChar *)"system-identifier"; /* Present systemId */

    /* This call will pass a NULL pointer to fprintf with a %s specifier, triggering UB
     * and typically a null-pointer-dereference crash on many libc implementations. */
    notationDeclDebug(NULL, name, publicId, systemId);

    /* Also try with systemId NULL to increase likelihood of hitting the bug across libcs. */
    publicId = (const xmlChar *)"public-identifier";
    systemId = NULL; /* Absent systemId */
    notationDeclDebug(NULL, name, publicId, systemId);

    return 0;
}
