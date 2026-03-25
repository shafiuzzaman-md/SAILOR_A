// Standalone reproducer for the rngTestStreaming use-after-free
// It stubs a tiny subset of the libxml2 Relax NG API and emulates the bug
// where a parser context retains a pointer to a freed xmlDoc.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Minimal type aliases mimicking libxml2 public API types
typedef unsigned char xmlChar;

typedef struct _xmlDoc {
    uint32_t magic;
    char *content;
} xmlDoc, *xmlDocPtr;

typedef struct _xmlRelaxNGParserCtxt {
    xmlDocPtr doc; // retains pointer to schema_doc
} xmlRelaxNGParserCtxt, *xmlRelaxNGParserCtxtPtr;

typedef struct _xmlRelaxNG {
    int dummy;
} xmlRelaxNG, *xmlRelaxNGPtr;

// --- Stubbed public API ---
int xmlMemUsed(void) {
    return 0;
}

// Simulate reading a file into an xmlDoc
xmlDocPtr xmlReadFile(const char *filename, const char *encoding, int options) {
    (void)encoding; (void)options;
    xmlDocPtr d = (xmlDocPtr)malloc(sizeof(*d));
    if (!d) return NULL;
    d->magic = 0xDEADBEEF;
    const char *payload = "<schema/>"; // minimal dummy content
    d->content = (char*)malloc(strlen(payload) + 1);
    if (!d->content) {
        free(d);
        return NULL;
    }
    strcpy(d->content, payload);
    fprintf(stderr, "xmlReadFile: allocated xmlDoc %p (content %p) for '%s'\n", (void*)d, (void*)d->content, filename);
    return d;
}

// Create a parser context that keeps a pointer to the provided xmlDoc
xmlRelaxNGParserCtxtPtr xmlRelaxNGNewDocParserCtxt(xmlDocPtr doc) {
    xmlRelaxNGParserCtxtPtr ctx = (xmlRelaxNGParserCtxtPtr)malloc(sizeof(*ctx));
    if (!ctx) return NULL;
    ctx->doc = doc; // BUG pattern: retains pointer to external doc
    fprintf(stderr, "xmlRelaxNGNewDocParserCtxt: ctx %p retains doc %p\n", (void*)ctx, (void*)doc);
    return ctx;
}

void xmlFreeDoc(xmlDocPtr doc) {
    if (!doc) return;
    fprintf(stderr, "xmlFreeDoc: freeing xmlDoc %p (content %p)\n", (void*)doc, (void*)doc->content);
    free(doc->content);
    free(doc);
}

// Parsing function that (intentionally) dereferences the retained doc pointer.
// If the doc was freed, this is a heap-use-after-free and ASan will flag it.
xmlRelaxNGPtr xmlRelaxNGParse(xmlRelaxNGParserCtxtPtr ctx) {
    if (!ctx) return NULL;
    // Volatile to prevent the compiler from optimizing the read away
    volatile uint32_t observed_magic = 0;
    // This access is the problematic dereference of potentially freed memory
    observed_magic = ctx->doc->magic; // UAF if ctx->doc was freed
    fprintf(stderr, "xmlRelaxNGParse: observed magic=0x%08x from doc %p\n", observed_magic, (void*)ctx->doc);

    // Return a dummy schema object
    xmlRelaxNGPtr schema = (xmlRelaxNGPtr)malloc(sizeof(*schema));
    if (schema) schema->dummy = 1;
    return schema;
}

void xmlRelaxNGFreeParserCtxt(xmlRelaxNGParserCtxtPtr ctx) {
    fprintf(stderr, "xmlRelaxNGFreeParserCtxt: freeing ctx %p (doc pointer kept as %p)\n", (void*)ctx, ctx ? (void*)ctx->doc : NULL);
    free(ctx);
}

void xmlRelaxNGFree(xmlRelaxNGPtr schema) {
    fprintf(stderr, "xmlRelaxNGFree: freeing schema %p\n", (void*)schema);
    free(schema);
}

int main(void) {
    // This follows the vulnerable pattern from runsuite.c:rngTestStreaming
    int mem = xmlMemUsed();
    (void)mem;

    // Step 1: Read schema file (stubbed)
    xmlDocPtr schema_doc = xmlReadFile("test/relaxng/ISO19005-1-XMP_Packet.rng", NULL, 0);
    if (schema_doc == NULL) {
        fprintf(stderr, "Failed to parse schema file\n");
        return 1;
    }

    // Step 2: Create parser context from schema_doc
    xmlRelaxNGParserCtxtPtr rng_parser_ctx = xmlRelaxNGNewDocParserCtxt(schema_doc);
    if (rng_parser_ctx == NULL) {
        xmlFreeDoc(schema_doc);
        fprintf(stderr, "Failed to create Relax NG parser context\n");
        return 1;
    }

    // Step 3: BUG: Free schema_doc while the context still retains its pointer
    xmlFreeDoc(schema_doc);

    // Optional: Allocate some noise to increase the chance of reusing freed memory
    // (not necessary for ASan to detect the UAF, but harmless)
    void *noise = malloc(128);
    memset(noise, 0xA5, 128);

    // Step 4: Trigger use-after-free by parsing with the context
    xmlRelaxNGPtr schema = xmlRelaxNGParse(rng_parser_ctx);

    // Cleanup
    xmlRelaxNGFreeParserCtxt(rng_parser_ctx);
    xmlRelaxNGFree(schema);
    free(noise);

    fprintf(stderr, "Done. If compiled with -fsanitize=address, a heap-use-after-free should be reported above.\n");
    return 0;
}
