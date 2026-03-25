#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal types/enums to satisfy the vulnerable function */
typedef enum {
    XML_ERR_OK = 0,
    XML_ERR_ARGUMENT = 2
} xmlParserErrors;

typedef struct {
    xmlParserErrors code;
} xmlError;

static xmlError globalErr = { XML_ERR_OK };

/* Stub for libxml2 internal error accessor */
static xmlError *xmlGetLastErrorInternal(void) {
    return &globalErr;
}

/* xmlChar/BAD_CAST minimal definitions */
typedef unsigned char xmlChar;
#define BAD_CAST (const xmlChar *)

/* Stubs for catalog resolution functions (not reached due to crash earlier) */
static const xmlChar *xmlCatalogLocalResolve(void *localCatalogs, const xmlChar *pub, const xmlChar *sys) {
    (void)localCatalogs; (void)pub; (void)sys; return NULL;
}
static const xmlChar *xmlCatalogResolve(const xmlChar *pub, const xmlChar *sys) {
    (void)pub; (void)sys; return NULL;
}
static const xmlChar *xmlCatalogLocalResolveURI(void *localCatalogs, const xmlChar *uri) {
    (void)localCatalogs; (void)uri; return NULL;
}
static const xmlChar *xmlCatalogResolveURI(const xmlChar *uri) {
    (void)uri; return NULL;
}

/* Stub that dereferences its argument: passing NULL will crash */
static int xmlNoNetExists(const char *url) {
    /* Intentional null-deref when url == NULL to demonstrate the bug path */
    volatile char c = url[0];
    (void)c;
    return 0;
}

/* Vulnerable function copied/adapted from parserInternals.c */
static xmlParserErrors
xmlResolveFromCatalog(const char *url, const char *publicId,
                      void *localCatalogs, int allowGlobal, char **out) {
    xmlError oldError;
    xmlError *lastError;
    char *resource = NULL;
    xmlParserErrors code;

    if (out == NULL)
        return(XML_ERR_ARGUMENT);
    *out = NULL;
    if ((localCatalogs == NULL) && (!allowGlobal))
        return(XML_ERR_OK);

    /*
     * Don't try to resolve if local file exists.
     *
     * TODO: This is somewhat non-deterministic.
     */
    if (xmlNoNetExists(url))
        return(XML_ERR_OK);

    /* Backup and reset last error */
    lastError = xmlGetLastErrorInternal();
    oldError = *lastError;
    lastError->code = XML_ERR_OK;

    /*
     * Do a local lookup
     */
    if (localCatalogs != NULL) {
        resource = (char *) xmlCatalogLocalResolve(localCatalogs,
                                                   BAD_CAST publicId,
                                                   BAD_CAST url);
    }
    /*
     * Try a global lookup
     */
    if ((resource == NULL) && (allowGlobal)) {
        resource = (char *) xmlCatalogResolve(BAD_CAST publicId,
                                              BAD_CAST url);
    }

    /*
     * Try to resolve url using URI rules.
     */
    if ((resource == NULL) && (url != NULL)) {
        if (localCatalogs != NULL) {
            resource = (char *) xmlCatalogLocalResolveURI(localCatalogs,
                                                          BAD_CAST url);
        }
        if ((resource == NULL) && (allowGlobal)) {
            resource = (char *) xmlCatalogResolveURI(BAD_CAST url);
        }
    }

    (void)code; /* silence unused warning in this stub */
    *out = resource;
    return XML_ERR_OK;
}

int main(void) {
    /* Trigger path: pass url == NULL, allowGlobal true so we don't return early */
    char *out = NULL;
    /* publicId can be anything; localCatalogs is NULL, allowGlobal = 1 */
    (void)xmlResolveFromCatalog(NULL, "-//PUBLIC//ID", NULL, 1, &out);

    /* We should never reach here due to the crash in xmlNoNetExists */
    printf("Unexpectedly returned, out=%p\n", (void*)out);
    return 0;
}
