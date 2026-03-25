#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for libxml2 types/macros used by runsuite.c */
typedef unsigned char xmlChar;
#define BAD_CAST (xmlChar*)

/* Minimal XML node/doc structures and helpers */
typedef struct _xmlNode {
    xmlChar *name;
    struct _xmlNode *children;
    struct _xmlNode *next;
} xmlNode;

typedef struct _xmlDoc {
    xmlNode *root;
} xmlDoc;

static int xmlStrcmp(const xmlChar *str1, const xmlChar *str2) {
    return strcmp((const char *)str1, (const char *)str2);
}

static xmlDoc *xmlReadFile(const char *filename, const char *enc, int opts) {
    (void)filename; (void)enc; (void)opts;
    /* Build a minimal document with the structure:
       <xmpmeta><RDF><Description/></RDF></xmpmeta> */
    xmlDoc *doc = (xmlDoc *)calloc(1, sizeof(xmlDoc));
    if (!doc) return NULL;

    xmlNode *xmpmeta = (xmlNode *)calloc(1, sizeof(xmlNode));
    xmlNode *rdf = (xmlNode *)calloc(1, sizeof(xmlNode));
    xmlNode *description = (xmlNode *)calloc(1, sizeof(xmlNode));
    if (!xmpmeta || !rdf || !description) {
        free(xmpmeta); free(rdf); free(description); free(doc);
        return NULL;
    }

    xmpmeta->name = BAD_CAST "xmpmeta";
    rdf->name = BAD_CAST "RDF";
    description->name = BAD_CAST "Description";

    rdf->children = description;
    xmpmeta->children = rdf;
    doc->root = xmpmeta;
    return doc;
}

static xmlNode *xmlDocGetRootElement(xmlDoc *doc) {
    return doc ? doc->root : NULL;
}

static xmlNode *xmlFirstElementChild(xmlNode *node) {
    return node ? node->children : NULL;
}

static xmlNode *xmlNextElementSibling(xmlNode *node) {
    return node ? node->next : NULL;
}

static void xmlFreeDoc(xmlDoc *doc) {
    if (!doc) return;
    xmlNode *x = doc->root;
    if (x) {
        xmlNode *r = x->children;
        if (r) {
            xmlNode *d = r->children;
            free(d);
            free(r);
        }
        free(x);
    }
    free(doc);
}

/* Minimal RNG API stand-ins */
static void *xmlRelaxNGNewParserCtxt(const char *filename) {
    (void)filename;
    /* Return non-NULL so the buggy check passes, even if parsing fails later */
    return (void *)0x1;
}

static void xmlRelaxNGFreeParserCtxt(void *ctx) {
    (void)ctx;
}

static void *xmlRelaxNGParse(void *parser_ctx) {
    (void)parser_ctx;
    /* Simulate schema parse failure: return NULL schema */
    return NULL;
}

static void xmlRelaxNGFree(void *schema) {
    (void)schema;
}

static void *xmlRelaxNGNewValidCtxt(void *schema) {
    /* According to libxml2 semantics, a NULL schema leads to NULL validation ctx */
    if (schema == NULL) return NULL;
    return malloc(4); /* dummy */
}

static void xmlRelaxNGFreeValidCtxt(void *ctx) {
    free(ctx);
}

/* This function will dereference the validation context, causing a crash when ctx is NULL */
static int xmlRelaxNGValidatePushElement(void *validation_ctx, xmlDoc *doc, xmlNode *node) {
    (void)doc; (void)node;
    /* Force a NULL pointer dereference like libxml2 internals would when ctx is NULL */
    volatile int *p = (int *)validation_ctx;
    return *p; /* ASan will flag this as a null-pointer-dereference when ctx is NULL */
}

/* Stub to satisfy compilation if ever reached (it won't be before the crash) */
static int xmlRelaxNGValidatePopElement(void *validation_ctx, xmlDoc *doc, xmlNode *node) {
    (void)validation_ctx; (void)doc; (void)node; return 1;
}

/* Buggy function modeled after runsuite.c::rngTestStreaming with the incorrect NULL checks */
static int rngTestStreaming(const char *xmp_rnc_filepath, const char *xmp_packet_filepath) {
    void *rng_parser_ctx = NULL;
    void *schema = NULL;
    void *validation_ctx = NULL;
    xmlDoc *xmp_packet_doc = NULL;
    xmlNode *xmpmeta = NULL, *rdf = NULL, *description = NULL;
    int rc = 0;

    rng_parser_ctx = xmlRelaxNGNewParserCtxt(xmp_rnc_filepath);
    if (rng_parser_ctx == NULL) {
        fprintf(stderr, "RNG Streaming: Failed to create Relax NG parser context\n");
        return -1;
    }

    schema = xmlRelaxNGParse(rng_parser_ctx);
    /* BUG: The code should check 'schema == NULL' here but incorrectly checks 'rng_parser_ctx' */
    if (rng_parser_ctx == NULL) {
        xmlRelaxNGFreeParserCtxt(rng_parser_ctx);
        fprintf(stderr, "RNG Streaming: Failed to parse Relax NG schema: %s\n", xmp_rnc_filepath);
        return -1;
    }

    xmlRelaxNGFreeParserCtxt(rng_parser_ctx);
    validation_ctx = xmlRelaxNGNewValidCtxt(schema);
    /* BUG: The code should check 'validation_ctx == NULL' but incorrectly checks 'rng_parser_ctx' again */
    if (rng_parser_ctx == NULL) {
        xmlRelaxNGFree(schema);
        fprintf(stderr, "RNG Streaming: Failed to create Relax NG validation context\n");
        return -1;
    }

    xmp_packet_doc = xmlReadFile(xmp_packet_filepath, NULL, 0);
    if (xmp_packet_doc == NULL) {
        xmlRelaxNGFreeValidCtxt(validation_ctx);
        xmlRelaxNGFree(schema);
        fprintf(stderr, "RNG Streaming: Failed to parse %s\n", xmp_packet_filepath);
        return -1;
    }

    xmpmeta = xmlDocGetRootElement(xmp_packet_doc);
    if (xmpmeta == NULL || xmlStrcmp(BAD_CAST "xmpmeta", xmpmeta->name) != 0) {
        fprintf(stderr, "RNG Streaming: Unable to find <x:xmpmeta> element\n");
        rc = -1;
        goto Exit;
    }

    rdf = xmlFirstElementChild(xmpmeta);
    if (rdf == NULL || xmlStrcmp(BAD_CAST "RDF", rdf->name) != 0) {
        fprintf(stderr, "RNG Streaming: Unable to find <rdf:RDF> element\n");
        rc = -1;
        goto Exit;
    }

    description = xmlFirstElementChild(rdf);
    if (description == NULL || xmlStrcmp(BAD_CAST "Description", description->name) != 0) {
        fprintf(stderr, "RNG Streaming: Unable to find <rdf:Description> element\n");
        rc = -1;
        goto Exit;
    }

    /* This call dereferences validation_ctx. Due to the buggy checks above,
       validation_ctx is NULL and this triggers a NULL pointer dereference. */
    rc = xmlRelaxNGValidatePushElement(validation_ctx, xmp_packet_doc, xmpmeta);

Exit:
    xmlFreeDoc(xmp_packet_doc);
    xmlRelaxNGFreeValidCtxt(validation_ctx);
    xmlRelaxNGFree(schema);
    return rc;
}

int main(void) {
    /* Filenames are irrelevant for this stub; the stubs ignore them. */
    (void)rngTestStreaming("/nonexistent/schema.rng", "/nonexistent/xmp_packet.xml");
    return 0;
}
