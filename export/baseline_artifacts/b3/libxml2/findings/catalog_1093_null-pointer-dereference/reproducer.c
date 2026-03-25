#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal libxml2-like typedefs */
typedef unsigned char xmlChar;

/* Stub: whitespace check similar to libxml2's xmlIsBlank_ch */
static int xmlIsBlank_ch(unsigned int c) {
    return (c == 0x20 || c == 0x9 || c == 0xA || c == 0xD);
}

/* Stub: simulate allocation failure in xmlStrdup */
static xmlChar *xmlStrdup(const xmlChar *cur) {
    (void)cur;
    /* Simulate out-of-memory: return NULL to trigger the bug */
    return NULL;
}

/* Vulnerable function (adapted from catalog.c) */
static xmlChar *
xmlCatalogNormalizePublic(const xmlChar *pubID)
{
    int ok = 1;
    int white;
    const xmlChar *p;
    xmlChar *ret;
    xmlChar *q;

    if (pubID == NULL)
        return(NULL);

    white = 1;
    for (p = pubID; *p != 0 && ok; p++) {
        if (!xmlIsBlank_ch(*p))
            white = 0;
        else if (*p == 0x20 && !white)
            white = 1;
        else
            ok = 0;
    }
    if (ok && !white) /* is normalized */
        return(NULL);

    ret = xmlStrdup(pubID); /* returns NULL in our stub */
    q = ret;
    white = 0;
    for (p = pubID; *p != 0; p++) {
        if (xmlIsBlank_ch(*p)) {
            if (q != ret)
                white = 1;
        } else {
            if (white) {
                *(q++) = 0x20; /* write via NULL q if reached */
                white = 0;
            }
            *(q++) = *p;       /* write via NULL q if reached */
        }
    }
    *q = 0; /* NULL pointer dereference when pubID is empty */
    return(ret);
}

int main(void) {
    /* Use an empty public ID so the inner loop doesn't run and *q=0 executes */
    const xmlChar *pubID = (const xmlChar *)"";

    /* This call will crash due to *q = 0 with q == NULL */
    xmlChar *res = xmlCatalogNormalizePublic(pubID);

    /* Not reached, but keep to satisfy return type */
    if (res) free(res);
    return 0;
}
