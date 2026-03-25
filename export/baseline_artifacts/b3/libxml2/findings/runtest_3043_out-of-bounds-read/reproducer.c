#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

/* Stubs and minimal types to avoid linking libxml2 */
typedef struct _xmlDoc { int dummy; } xmlDoc;
typedef xmlDoc* xmlDocPtr;
static xmlDocPtr xpathDocument;

#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

#ifndef XML_PARSE_DTDATTR
#define XML_PARSE_DTDATTR 0
#endif
#ifndef XML_PARSE_NOENT
#define XML_PARSE_NOENT 0
#endif

/* Stub xmlReadFile: return a non-NULL doc so the vulnerable path runs */
xmlDocPtr xmlReadFile(const char *filename, const char *enc, int opts) {
    (void)filename; (void)enc; (void)opts;
    static xmlDoc dummy;
    return &dummy;
}

/* Stub xmlFreeDoc: no-op */
void xmlFreeDoc(xmlDocPtr doc) { (void)doc; }

/* Stub xpathCommonTest: return success */
int xpathCommonTest(const char *file, const char *result, int a, int b) {
    (void)file; (void)result; (void)a; (void)b; return 0;
}

/* baseFilename equivalent: returns pointer to last path component.
 * Intentionally reads the input as a C string. */
static const char *baseFilename(const char *filename) {
    if (filename == NULL)
        return "";
    const char *base = filename;
    for (const char *p = filename; *p; p++) {
        if (*p == '/' || *p == '\\')
            base = p + 1;
    }
    return base;
}

/* Interpose glob and globfree to simulate the bug reliably.
 * Our glob() returns GLOB_NOMATCH and intentionally does NOT touch pglob,
 * leaving its fields as-is (unspecified/garbage), mirroring the problematic
 * scenario from the original code when return value isn't checked. */
int glob(const char *pattern, int flags,
         int (*errfunc)(const char *epath, int eerrno),
         glob_t *pglob) {
    (void)pattern; (void)flags; (void)errfunc; (void)pglob;
    return GLOB_NOMATCH;
}

void globfree(glob_t *pglob) { (void)pglob; }

/* Vulnerable logic adapted from runtest.c:xptrDocTest */
static int xptrDocTest(const char *filename,
                       const char *resul ATTRIBUTE_UNUSED,
                       const char *err ATTRIBUTE_UNUSED,
                       int options ATTRIBUTE_UNUSED) {
    char pattern[500];
    char result[500];
    glob_t globbuf;
    size_t i;
    int ret = 0, res;

    xpathDocument = xmlReadFile(filename, NULL,
                                XML_PARSE_DTDATTR | XML_PARSE_NOENT);
    if (xpathDocument == NULL) {
        fprintf(stderr, "Failed to load %s\n", filename);
        return -1;
    }

    /* Prepare a crafted gl_pathv entry that is a non-NUL-terminated buffer
     * so baseFilename will read out-of-bounds. */
    char *non_null_terminated = (char *)malloc(1);
    if (!non_null_terminated) {
        fprintf(stderr, "malloc failed\n");
        return -1;
    }
    non_null_terminated[0] = 'A';
    char *evilv[1];
    evilv[0] = non_null_terminated; /* Points to 1-byte buffer with no NUL */

    /* Initialize globbuf with dangerous values that our interposed glob() will
     * not modify (contents remain unspecified on error). */
    memset(&globbuf, 0xCD, sizeof(globbuf)); /* poison to emphasize undefined */
    globbuf.gl_offs = 0;
    globbuf.gl_pathc = 1;        /* Pretend there's 1 match */
    globbuf.gl_pathv = evilv;    /* Array containing our crafted pointer */

    res = snprintf(pattern, 499, "./test/XPath/xptr/%s*",
                   baseFilename(filename));
    if (res >= 499)
        pattern[499] = 0;

    /* glob() fails with GLOB_NOMATCH and leaves globbuf as-is. */
    (void)glob(pattern, GLOB_DOOFFS, NULL, &globbuf);

    /* Vulnerable loop: uses globbuf fields without checking glob()'s return. */
    for (i = 0; i < globbuf.gl_pathc; i++) {
        res = snprintf(result, 499, "result/XPath/xptr/%s",
                       baseFilename(globbuf.gl_pathv[i]));
        if (res >= 499)
            result[499] = 0;
        /* Normally would run tests here; with our crafted input, we should
         * have already triggered an ASan OOB read in baseFilename. */
        res = xpathCommonTest(globbuf.gl_pathv[i], &result[0], 1, 0);
        if (res != 0)
            ret = res;
    }

    globfree(&globbuf);
    xmlFreeDoc(xpathDocument);
    return ret;
}

int main(void) {
    /* Any filename; xmlReadFile is stubbed to succeed. */
    (void)xptrDocTest("dummy.xml", NULL, NULL, 0);
    /* If we didn't crash yet, indicate completion. */
    puts("Completed without crash (unexpected)");
    return 0;
}
