#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-in for libxml2's xmlChar */
typedef unsigned char xmlChar;

/*
 * Minimal reproduction of libxml2's xmlCheckLanguageID vulnerable logic
 * including the region_m49 block that reads nxt[1] and nxt[2] unconditionally.
 */
int xmlCheckLanguageID(const xmlChar *lang) {
    const xmlChar *cur = lang;
    const xmlChar *nxt = cur;

    /* parse primary language subtag: 1-8 alpha */
    if (!(((nxt[0] >= 'A') && (nxt[0] <= 'Z')) ||
          ((nxt[0] >= 'a') && (nxt[0] <= 'z'))))
        return 0;

    while (((nxt[0] >= 'A') && (nxt[0] <= 'Z')) ||
           ((nxt[0] >= 'a') && (nxt[0] <= 'z')))
        nxt++;

    if ((nxt - cur < 1) || (nxt - cur > 8))
        return 0;

    if (nxt[0] == 0)
        return 1;
    if (nxt[0] != '-')
        return 0;

    nxt++;
    cur = nxt;

    /* now we can have region or variant */
    if ((nxt[0] >= '0') && (nxt[0] <= '9'))
        goto region_m49;

    while (((nxt[0] >= 'A') && (nxt[0] <= 'Z')) ||
           ((nxt[0] >= 'a') && (nxt[0] <= 'z')))
        nxt++;

    if ((nxt - cur >= 5) && (nxt - cur <= 8))
        goto variant;
    if (nxt - cur != 2)
        return 0;
    /* we parsed a region */
region:
    if (nxt[0] == 0)
        return 1;
    if (nxt[0] != '-')
        return 0;

    nxt++;
    cur = nxt;
    /* now we can just have a variant */
    while (((nxt[0] >= 'A') && (nxt[0] <= 'Z')) ||
           ((nxt[0] >= 'a') && (nxt[0] <= 'z')))
        nxt++;

    if ((nxt - cur < 5) || (nxt - cur > 8))
        return 0;

    /* we parsed a variant */
variant:
    if (nxt[0] == 0)
        return 1;
    if (nxt[0] != '-')
        return 0;
    /* extensions and private use subtags not checked */
    return 1;

region_m49:
    /* Vulnerable read: assumes there are at least 3 digits available. */
    if (((nxt[1] >= '0') && (nxt[1] <= '9')) &&
        ((nxt[2] >= '0') && (nxt[2] <= '9'))) {
        nxt += 3;
        goto region;
    }
    return 0;
}

int main(void) {
    /* Use heap allocation so ASan redzones catch the out-of-bounds read. */
    const char *lit = "en-1"; /* primary=\"en\", then a single digit to trigger region_m49 */
    size_t len = strlen(lit);
    xmlChar *buf = (xmlChar *)malloc(len + 1);
    if (!buf) {
        perror("malloc");
        return 1;
    }
    memcpy(buf, lit, len + 1);

    /* This call will read buf[5] (one byte past the NUL at buf[4]) in region_m49. */
    int ret = xmlCheckLanguageID(buf);

    printf("xmlCheckLanguageID(\"%s\") returned %d\n", (char *)buf, ret);

    free(buf);
    return 0;
}
