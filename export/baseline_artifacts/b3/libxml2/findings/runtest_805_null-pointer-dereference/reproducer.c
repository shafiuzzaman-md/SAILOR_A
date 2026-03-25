#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal type/struct stubs to mirror libxml2 types used in runtest.c */
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar *)

typedef struct _xmlEntity {
    xmlChar *URI;
} xmlEntity, *xmlEntityPtr;

/* Debug context carrying the fields accessed by entityDeclDebug */
typedef struct {
    void *parameterEntities;
    void *generalEntities;
    const char *filename;
} debugContext;

/* Globals used by runtest.c debug callbacks */
static int quiet = 0;
static int callbacks = 0;
static FILE *SAXdebug = NULL;

/* Dummy constants to satisfy comparisons in entityDeclDebug */
#define XML_INTERNAL_PARAMETER_ENTITY 1
#define XML_EXTERNAL_PARAMETER_ENTITY 2

/* Stubbed helpers matching the signatures used in the vulnerable code */
static xmlEntityPtr xmlNewEntity(void *unused, const xmlChar *name, int type,
                                 const xmlChar *publicId, const xmlChar *systemId,
                                 xmlChar *content) {
    (void)unused; (void)name; (void)type; (void)publicId; (void)systemId; (void)content;
    /* Simulate allocation failure (e.g., OOM): return NULL to trigger the bug */
    return NULL;
}

static xmlChar *xmlBuildURI(const xmlChar *URI, const xmlChar *base) {
    (void)base; /* Base is unused in this stub */
    /* Just duplicate the input URI */
    size_t len = strlen((const char *)URI) + 1;
    xmlChar *out = (xmlChar *)malloc(len);
    if (out)
        memcpy(out, URI, len);
    return out;
}

static int xmlHashAddEntry(void *table, const xmlChar *name, void *userdata) {
    (void)table; (void)name; (void)userdata;
    return 0;
}

/* Vulnerable function adapted from runtest.c (line numbers omitted) */
static void entityDeclDebug(void *ctx, const xmlChar *name, int type,
                            const xmlChar *publicId, const xmlChar *systemId, xmlChar *content) {
    debugContext *ctxt = (debugContext *)ctx;
    xmlEntityPtr ent;
    const xmlChar *nullstr = BAD_CAST "(null)";

    ent = xmlNewEntity(NULL, name, type, publicId, systemId, content);
    if (systemId != NULL)
        /* BUG: ent may be NULL; this dereference causes a crash */
        ent->URI = xmlBuildURI(systemId, (const xmlChar *)ctxt->filename);

    if ((type == XML_INTERNAL_PARAMETER_ENTITY) ||
        (type == XML_EXTERNAL_PARAMETER_ENTITY))
        xmlHashAddEntry(ctxt->parameterEntities, name, ent);
    else
        xmlHashAddEntry(ctxt->generalEntities, name, ent);

    if (publicId == NULL)
        publicId = nullstr;
    if (systemId == NULL)
        systemId = nullstr;
    if (content == NULL)
        content = (xmlChar *)nullstr;
    callbacks++;
    if (quiet)
        return;
    fprintf(SAXdebug, "SAX.entityDecl(%s, %d, %s, %s, %s)\n",
            (const char *)name, type, (const char *)publicId, (const char *)systemId, (const char *)content);
}

int main(void) {
    SAXdebug = stdout;

    debugContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.filename = "base.xml"; /* used by xmlBuildURI */

    /* Provide a non-NULL systemId so the buggy path is taken */
    const xmlChar *name = BAD_CAST "myEntity";
    const xmlChar *publicId = NULL;
    const xmlChar *systemId = BAD_CAST "http://example.com/ext.ent";
    xmlChar *content = BAD_CAST "contents";

    /* This call will NULL-deref inside entityDeclDebug due to xmlNewEntity() returning NULL */
    entityDeclDebug(&ctx, name, 0, publicId, systemId, content);

    /* We should never reach here */
    fprintf(stderr, "ERROR: did not crash as expected\n");
    return 0;
}
