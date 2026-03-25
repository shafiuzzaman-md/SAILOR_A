#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/*
 * Self-contained minimal reproduction of the libxml2 bug:
 * xmlNewCharEncodingHandler returns a pointer even if xmlRegisterCharEncodingHandler
 * frees it on failure (e.g., when MAX_ENCODING_HANDLERS is exceeded), leading to
 * a use-after-free when the caller uses the returned pointer.
 */

/* Simplified handler flags from libxml2 */
#define XML_HANDLER_STATIC 1
#define XML_HANDLER_LEGACY 2

/* Keep this small so we can easily exceed the limit and trigger the bug */
#define MAX_ENCODING_HANDLERS 8

/* Forward decls matching (simplified) public API */
typedef int (*xmlCharEncodingInputFunc)(unsigned char *out, int outlen,
                                        const unsigned char *in, int inlen);
typedef int (*xmlCharEncodingOutputFunc)(unsigned char *out, int outlen,
                                         const unsigned char *in, int inlen);

typedef struct _xmlCharEncodingHandler xmlCharEncodingHandler;
typedef xmlCharEncodingHandler *xmlCharEncodingHandlerPtr;

struct _xmlCharEncodingHandler {
    union { xmlCharEncodingInputFunc legacyFunc; void *func; } input;
    union { xmlCharEncodingOutputFunc legacyFunc; void *func; } output;
    char *name;
    int flags;
};

/* Global registry simulation (simplified version of libxml2's global handlers) */
static xmlCharEncodingHandlerPtr globalHandlers[MAX_ENCODING_HANDLERS];
static int nbCharEncodingHandler = 0;

/* Minimal memory wrappers used by libxml2 code */
static void *xmlMalloc(size_t sz) { return malloc(sz); }
static void xmlFree(void *p) { free(p); }
static char *xmlMemStrdup(const char *s) { return s ? strdup(s) : NULL; }

/* Alias resolution used by xmlNewCharEncodingHandler; return NULL (no alias) */
static const char *xmlGetEncodingAlias(const char *name) {
    (void)name;
    return NULL;
}

/*
 * Vulnerable public API (legacy variant) simplified from libxml2 encoding.c:
 * - Builds an uppercase name copy
 * - Allocates handler, assigns fields
 * - Calls xmlRegisterCharEncodingHandler(handler)
 * - Unconditionally returns handler (BUG: handler may have been freed).
 */
static void xmlRegisterCharEncodingHandler(xmlCharEncodingHandlerPtr handler);

xmlCharEncodingHandlerPtr
xmlNewCharEncodingHandler(const char *name,
                          xmlCharEncodingInputFunc input,
                          xmlCharEncodingOutputFunc output) {
    char upper[500];
    int i;
    char *up;
    xmlCharEncodingHandlerPtr handler;
    const char *alias;

    alias = xmlGetEncodingAlias(name);
    if (alias != NULL)
        name = alias;

    if (name == NULL)
        return NULL;

    for (i = 0; i < 499; i++) {
        upper[i] = (char)toupper((unsigned char)name[i]);
        if (upper[i] == 0) break;
    }
    upper[i] = 0;

    up = xmlMemStrdup(upper);
    if (up == NULL)
        return NULL;

    handler = (xmlCharEncodingHandlerPtr)xmlMalloc(sizeof(xmlCharEncodingHandler));
    if (handler == NULL) {
        xmlFree(up);
        return NULL;
    }
    memset(handler, 0, sizeof(xmlCharEncodingHandler));
    handler->input.legacyFunc = input;
    handler->output.legacyFunc = output;
    handler->name = up;
    handler->flags = XML_HANDLER_STATIC | XML_HANDLER_LEGACY;

    /* Register the handler; on failure, the register function may free it. */
    xmlRegisterCharEncodingHandler(handler);

    /* BUG: returns handler even if it was freed by xmlRegisterCharEncodingHandler */
    return handler;
}

/*
 * Simplified version of libxml2's xmlRegisterCharEncodingHandler.
 * When the registry is full, it frees the passed-in handler and returns,
 * leaving the caller (xmlNewCharEncodingHandler) with a dangling pointer.
 */
static void xmlRegisterCharEncodingHandler(xmlCharEncodingHandlerPtr handler) {
    if (handler == NULL)
        return;

    if (nbCharEncodingHandler >= MAX_ENCODING_HANDLERS) {
        /* Failure path: simulate libxml2 freeing the handler on error */
        fprintf(stderr, "[xmlRegister] Registry full, freeing handler '%s'\n",
                handler->name ? handler->name : "(null)");
        xmlFree(handler->name);
        xmlFree(handler);
        return;
    }

    globalHandlers[nbCharEncodingHandler++] = handler;
}

/* Dummy conversion callbacks matching our typedefs */
static int dummy_input(unsigned char *out, int outlen,
                       const unsigned char *in, int inlen) {
    (void)out; (void)outlen; (void)in; (void)inlen; return 0;
}

static int dummy_output(unsigned char *out, int outlen,
                        const unsigned char *in, int inlen) {
    (void)out; (void)outlen; (void)in; (void)inlen; return 0;
}

int main(void) {
    /* Fill the registry to capacity */
    for (int i = 0; i < MAX_ENCODING_HANDLERS; i++) {
        char name[32];
        snprintf(name, sizeof(name), "enc%d", i);
        xmlCharEncodingHandlerPtr h =
            xmlNewCharEncodingHandler(name, dummy_input, dummy_output);
        if (h == NULL) {
            fprintf(stderr, "Failed to create handler %d\n", i);
            return 1;
        }
        fprintf(stderr, "Registered handler %p with name %s\n", (void*)h, h->name);
    }

    /* This call will exceed the limit; xmlRegister frees the handler,
     * but xmlNewCharEncodingHandler still returns the freed pointer. */
    xmlCharEncodingHandlerPtr uaf =
        xmlNewCharEncodingHandler("trigger", dummy_input, dummy_output);

    fprintf(stderr, "xmlNewCharEncodingHandler returned %p (expected dangling)\n", (void*)uaf);

    /* Use-after-free: write to the freed handler memory. ASan should flag this. */
    if (uaf) {
        fprintf(stderr, "About to write to freed handler->flags to trigger UAF...\n");
        uaf->flags = 0x12345678; /* Heap-use-after-free */

        /* Also attempt to free it again to highlight the double-free possibility */
        fprintf(stderr, "About to free the already-freed handler to show double-free...\n");
        free(uaf);
    }

    return 0;
}
