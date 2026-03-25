#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for libxml2 types and APIs */
typedef unsigned char xmlChar;

typedef struct _xmlParserCtxt {
    int options;
} xmlParserCtxt, *xmlParserCtxtPtr;

typedef struct _xmlSBuf {
    char *data;
    size_t len;
    size_t cap;
} xmlSBuf;

/* Options/macro stubs */
#define XML_PARSE_HUGE 0x1
#define PARSER_STOPPED(ctxt) (0)

/* Stub helper functions that won't be reached before the OOB */
static int xmlUTF8MultibyteLen(xmlParserCtxtPtr ctxt, const xmlChar *str, const char *msg) {
    (void)ctxt; (void)str; (void)msg;
    return 1;
}

static void xmlSBufAddString(xmlSBuf *buf, const xmlChar *str, size_t len) {
    (void)buf; (void)str; (void)len;
}

static void xmlSBufAddReplChar(xmlSBuf *buf) {
    (void)buf;
}

static void xmlSBufAddChar(xmlSBuf *buf, int c) {
    (void)buf; (void)c;
}

static void xmlFatalErrMsg(xmlParserCtxtPtr ctxt, int err, const char *msg) {
    (void)ctxt; (void)err;
    fprintf(stderr, "xmlFatalErrMsg: %s\n", msg);
}

static int xmlParseStringCharRef(xmlParserCtxtPtr ctxt, const xmlChar **str) {
    (void)ctxt; (void)str;
    return 'a';
}

static xmlChar *xmlParseStringName(xmlParserCtxtPtr ctxt, const xmlChar **str) {
    (void)ctxt; (void)str;
    return NULL;
}

typedef struct _xmlEntity *xmlEntityPtr;
static xmlEntityPtr xmlParseStringPEReference(xmlParserCtxtPtr ctxt, const xmlChar **str) {
    (void)ctxt; (void)str;
    return NULL;
}

static void xmlFree(void *p) { free(p); }

/* Vulnerable function copied and minimally adapted */
static void
xmlExpandPEsInEntityValue(xmlParserCtxtPtr ctxt, xmlSBuf *buf,
                          const xmlChar *str, int length, int depth) {
    int maxDepth = (ctxt->options & XML_PARSE_HUGE) ? 40 : 20;
    const xmlChar *end, *chunk;
    int c, l;

    if (str == NULL)
        return;

    depth += 1;
    if (depth > maxDepth) {
        xmlFatalErrMsg(ctxt, 0,
                       "Maximum entity nesting depth exceeded");
        return;
    }

    end = str + length;
    chunk = str;

    while ((str < end) && (!PARSER_STOPPED(ctxt))) {
        c = *str;

        if (c >= 0x80) {
            l = xmlUTF8MultibyteLen(ctxt, str,
                    "invalid character in entity value\n");
            if (l == 0) {
                if (chunk < str)
                    xmlSBufAddString(buf, chunk, (size_t)(str - chunk));
                xmlSBufAddReplChar(buf);
                str += 1;
                chunk = str;
            } else {
                str += l;
            }
        } else if (c == '&') {
            /* BUG: If str == end - 1, str[1] reads 1 byte past end */
            if (str[1] == '#') {
                if (chunk < str)
                    xmlSBufAddString(buf, chunk, (size_t)(str - chunk));

                c = xmlParseStringCharRef(ctxt, &str);
                if (c == 0)
                    return;

                xmlSBufAddChar(buf, c);

                chunk = str;
            } else {
                xmlChar *name;

                /* General entity references are checked for syntactic validity. */
                str++;
                name = xmlParseStringName(ctxt, &str);

                if ((name == NULL) || (*str++ != ';')) {
                    xmlFatalErrMsg(ctxt, 0,
                            "EntityValue: '&' forbidden except for entities references\n");
                    xmlFree(name);
                    return;
                }

                xmlFree(name);
            }
        } else if (c == '%') {
            xmlEntityPtr ent;

            if (chunk < str)
                xmlSBufAddString(buf, chunk, (size_t)(str - chunk));

            ent = xmlParseStringPEReference(ctxt, &str);
            if (ent == NULL)
                return;
        } else {
            /* Skip normal ASCII */
            str++;
        }
    }
}

int main(void) {
    /* Construct an entity value that ends with '&' to trigger OOB read of str[1] */
    size_t len = 1;
    xmlChar *data = (xmlChar *)malloc(len);
    if (!data) {
        perror("malloc");
        return 1;
    }
    data[0] = (xmlChar)'&';

    xmlParserCtxt ctxt;
    ctxt.options = 0; /* not XML_PARSE_HUGE; irrelevant for this repro */

    xmlSBuf buf;
    buf.data = NULL; buf.len = 0; buf.cap = 0;

    /* depth = 0; length = 1; last byte is '&' */
    xmlExpandPEsInEntityValue(&ctxt, &buf, data, (int)len, 0);

    free(data);
    return 0;
}
