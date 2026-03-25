// Standalone reproducer for null-pointer-dereference in htmlCtxtSetOptionsInternal
// Compile with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

// Minimal stubs and typedefs to emulate the needed bits of libxml2

typedef unsigned char xmlChar;

typedef struct _xmlDict xmlDict;
struct _xmlDict { int dummy; };

// SAX callback type for ignorableWhitespace
typedef void (*ignorableWhitespaceSAXFunc)(void *ctx, const xmlChar *ch, int len);

// Minimal SAX handler struct with only the field we touch in the bug
typedef struct _xmlSAXHandler {
    ignorableWhitespaceSAXFunc ignorableWhitespace;
} xmlSAXHandler, *xmlSAXHandlerPtr;

// Minimal parser context with only the fields referenced in the vulnerable code
typedef struct _xmlParserCtxt {
    int options;
    int keepBlanks;
    int recovery;
    int dictNames;
    xmlSAXHandlerPtr sax;   // will be NULL to trigger the bug
    xmlDict *dict;
} xmlParserCtxt, *xmlParserCtxtPtr;

// Option bit masks (values don't need to match real libxml2; they just need to be distinct)
#define HTML_PARSE_RECOVER      (1 << 0)
#define HTML_PARSE_HTML5        (1 << 1)
#define HTML_PARSE_NODEFDTD     (1 << 2)
#define HTML_PARSE_NOERROR      (1 << 3)
#define HTML_PARSE_NOWARNING    (1 << 4)
#define HTML_PARSE_PEDANTIC     (1 << 5)
#define HTML_PARSE_NOBLANKS     (1 << 6)
#define HTML_PARSE_NONET        (1 << 7)
#define HTML_PARSE_NOIMPLIED    (1 << 8)
#define HTML_PARSE_COMPACT      (1 << 9)
#define HTML_PARSE_HUGE         (1 << 10)
#define HTML_PARSE_IGNORE_ENC   (1 << 11)
#define HTML_PARSE_BIG_LINES    (1 << 12)
#define XML_PARSE_NOENT         (1 << 13)

// Stub implementations for external functions referenced by the vulnerable code
void xmlSAX2IgnorableWhitespace(void *ctx, const xmlChar *ch, int len) {
    (void)ctx; (void)ch; (void)len;
}

void xmlDictSetLimit(xmlDict *dict, size_t limit) {
    (void)dict; (void)limit;
}

// Vulnerable function reproduced from the source context (trimmed to essentials)
static int htmlCtxtSetOptionsInternal(xmlParserCtxtPtr ctxt, int options, int keepMask)
{
    int allMask;

    if (ctxt == NULL)
        return(-1);

    allMask = HTML_PARSE_RECOVER |
              HTML_PARSE_HTML5 |
              HTML_PARSE_NODEFDTD |
              HTML_PARSE_NOERROR |
              HTML_PARSE_NOWARNING |
              HTML_PARSE_PEDANTIC |
              HTML_PARSE_NOBLANKS |
              HTML_PARSE_NONET |
              HTML_PARSE_NOIMPLIED |
              HTML_PARSE_COMPACT |
              HTML_PARSE_HUGE |
              HTML_PARSE_IGNORE_ENC |
              HTML_PARSE_BIG_LINES;

    ctxt->options = (ctxt->options & keepMask) | (options & allMask);

    // For some options, struct members are historically the source of truth
    ctxt->keepBlanks = (options & HTML_PARSE_NOBLANKS) ? 0 : 1;

    // Recover from character encoding errors
    ctxt->recovery = 1;

    // Changing SAX callbacks is a bad idea. This should be fixed.
    if (options & HTML_PARSE_NOBLANKS) {
        // BUG: No NULL check on ctxt->sax before dereferencing
        ctxt->sax->ignorableWhitespace = xmlSAX2IgnorableWhitespace; // NULL deref here when sax == NULL
    }
    if (options & HTML_PARSE_HUGE) {
        if (ctxt->dict != NULL)
            xmlDictSetLimit(ctxt->dict, 0);
    }

    // It would be useful to allow this feature.
    ctxt->dictNames = 0;

    // Allow XML_PARSE_NOENT which many users set on the HTML parser.
    return(options & ~allMask & ~XML_PARSE_NOENT);
}

// Public wrapper mirroring the real API
int htmlCtxtSetOptions(xmlParserCtxt *ctxt, int options)
{
    return htmlCtxtSetOptionsInternal(ctxt, options, 0);
}

int main(void)
{
    // Create a parser context with a NULL SAX handler to trigger the bug
    xmlParserCtxt ctxt;
    memset(&ctxt, 0, sizeof(ctxt));
    ctxt.sax = NULL;  // critical condition for the NULL dereference
    ctxt.dict = NULL;

    // Set the NOBLANKS option, which will cause the vulnerable code path to run
    int options = HTML_PARSE_NOBLANKS;

    // This call will crash with a NULL pointer dereference in AddressSanitizer
    // at the line where ctxt->sax->ignorableWhitespace is assigned.
    (void)htmlCtxtSetOptions(&ctxt, options);

    // We should never reach here
    printf("If you see this, the bug did not trigger as expected.\n");
    return 0;
}
