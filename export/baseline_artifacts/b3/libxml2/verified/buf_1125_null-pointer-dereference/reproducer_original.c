// Standalone reproducer for null-pointer-dereference in xmlBufferWriteQuotedString
// Compile with:
//   clang -fsanitize=address -g -O0 reproducer.c -o reproducer

#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Minimal re-declarations to mimic libxml2 types/APIs
typedef unsigned char xmlChar;

typedef struct _xmlBuffer {
    int dummy;
} xmlBuffer;

#define BAD_CAST (const xmlChar *)

// Buggy xmlStrchr: emulate libxml2 behavior by delegating to strchr without NULL checks
static const xmlChar *xmlStrchr(const xmlChar *str, xmlChar val) {
    // This will dereference NULL if 'str' is NULL, matching the vulnerability scenario
    const char *ret = strchr((const char *)str, (int)val);
    return (const xmlChar *)ret;
}

// Stubs for buffer manipulation used by xmlBufferWriteQuotedString
static void xmlBufferAdd(xmlBuffer *buf, const xmlChar *string, int len) {
    (void)buf; (void)string; (void)len; /* no-op */
}

static void xmlBufferCCat(xmlBuffer *buf, const char *string) {
    (void)buf; (void)string; /* no-op */
}

static void xmlBufferCat(xmlBuffer *buf, const xmlChar *string) {
    (void)buf; (void)string; /* no-op */
}

// Vulnerable function copied/adapted from buf.c
static void xmlBufferWriteQuotedString(xmlBuffer *buf, const xmlChar *string) {
    const xmlChar *cur, *base;
    if (buf == NULL)
        return;
    if (xmlStrchr(string, '"')) {    // BUG: no NULL-check on 'string' before xmlStrchr
        if (xmlStrchr(string, '\'')) {
            xmlBufferCCat(buf, "\"");
            base = cur = string;
            while (*cur != 0) {
                if (*cur == '"') {
                    if (base != cur)
                        xmlBufferAdd(buf, base, (int)(cur - base));
                    xmlBufferAdd(buf, BAD_CAST "&quot;", 6);
                    cur++;
                    base = cur;
                } else {
                    cur++;
                }
            }
            if (base != cur)
                xmlBufferAdd(buf, base, (int)(cur - base));
            xmlBufferCCat(buf, "\"");
        } else {
            xmlBufferCCat(buf, "'\0" + 0); // same as xmlBufferCCat(buf, "'") but avoids escape confusion
            xmlBufferCat(buf, string);
            xmlBufferCCat(buf, "'\0" + 0);
        }
    } else {
        xmlBufferCCat(buf, "\"");
        xmlBufferCat(buf, string);
        xmlBufferCCat(buf, "\"");
    }
}

int main(void) {
    // Prepare a non-NULL buffer so the function proceeds to the vulnerable path
    xmlBuffer buf = {0};

    // Trigger: pass NULL for 'string'
    const xmlChar *input = NULL;

    // This call will attempt to evaluate xmlStrchr(NULL, '"'), causing a NULL dereference
    xmlBufferWriteQuotedString(&buf, input);

    // If the bug does not trigger (should not happen), print a message
    puts("Unexpectedly survived null dereference");
    return 0;
}
