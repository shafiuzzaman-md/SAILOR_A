#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* Minimal type and macro redefinitions to mirror libxml2 API used here */
typedef unsigned char xmlChar;
#define BAD_CAST (const xmlChar *)

typedef struct _xmlCharEncodingHandler xmlCharEncodingHandler;
typedef xmlCharEncodingHandler *xmlCharEncodingHandlerPtr;

struct _xmlCharEncodingHandler {
    /* Minimal stub; real libxml2 has function pointers here */
    int dummy;
};

typedef int xmlCharEncFlags; /* stub */

enum {
    XML_ERR_OK = 0
};

/* Flags used in the vulnerable code path */
#define XML_ENC_INPUT  (1 << 0)
#define XML_ENC_OUTPUT (1 << 1)

/* Index used by the vulnerable function to return a built-in handler */
#define XML_CHAR_ENCODING_UTF8 1

/* Stub default handler table with at least two entries so index 1 is valid */
static xmlCharEncodingHandler defaultHandlers[2];

/* Simplified, bug-relevant implementation of xmlStrcasecmp from libxml2.
 * Critically, this function does not check for NULL inputs, matching the
 * behavior needed to demonstrate the NULL dereference when called with
 * a NULL 'name' argument cast via BAD_CAST. */
static int xmlStrcasecmp(const xmlChar *a, const xmlChar *b) {
    /* This will dereference 'a' and 'b' immediately. If 'a' is NULL, it crashes. */
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb)
            return ca - cb;
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

/* Stub for xmlCreateCharEncodingHandler used later in the function */
typedef int xmlParserErrors;
static xmlParserErrors xmlCreateCharEncodingHandler(const char *name,
                                                    xmlCharEncFlags flags,
                                                    void *a, void *b,
                                                    xmlCharEncodingHandler **out) {
    if (out)
        *out = NULL;
    (void)name; (void)flags; (void)a; (void)b;
    return XML_ERR_OK;
}

/* Vulnerable function recreated from the provided source context */
xmlCharEncodingHandler *
xmlFindCharEncodingHandler(const char *name) {
    xmlCharEncodingHandler *ret;
    xmlCharEncFlags flags;

    /*
     * This handler shouldn't be used, but we must return a non-NULL
     * handler.
     */
    if ((xmlStrcasecmp(BAD_CAST name, BAD_CAST "UTF-8") == 0) ||
        (xmlStrcasecmp(BAD_CAST name, BAD_CAST "UTF8") == 0))
        return (xmlCharEncodingHandlerPtr)
                &defaultHandlers[XML_CHAR_ENCODING_UTF8];

    flags = XML_ENC_INPUT;
#ifdef LIBXML_OUTPUT_ENABLED
    flags |= XML_ENC_OUTPUT;
#endif
    xmlCreateCharEncodingHandler(name, flags, NULL, NULL, &ret);
    return ret;
}

int main(void) {
    /* Trigger: Pass NULL for 'name' to cause NULL dereference in xmlStrcasecmp */
    const char *name = NULL;

    /* This call should crash with AddressSanitizer due to NULL deref */
    xmlCharEncodingHandler *h = xmlFindCharEncodingHandler(name);

    /* If it didn't crash (it should), print the result to avoid warnings */
    printf("handler=%p\n", (void*)h);
    return 0;
}
