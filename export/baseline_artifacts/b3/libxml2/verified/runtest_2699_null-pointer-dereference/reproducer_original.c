#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Emulate libxml2 test harness macros/types */
#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED
#endif

typedef struct _xmlTextReader xmlTextReader;
typedef xmlTextReader* xmlTextReaderPtr;

/* Globals expected by runtest.c */
int nb_tests = 0;
const char *temp_directory = ".";
char *testErrors = NULL;
size_t testErrorsSize = 0;

/* Stubs for external functions used by streamProcessTest */
static void fatalError(void) {
    /* In the real test harness, this would abort the test suite */
    fprintf(stderr, "fatalError called\n");
    exit(1);
}

static char *resultFilename(const char *filename, const char *dir, const char *ext) {
    /* Compose a temporary result filename (not used in this reproducer path) */
    size_t len = strlen(filename) + strlen(dir) + strlen(ext) + 2;
    char *out = (char *)malloc(len);
    if (!out) return NULL;
    snprintf(out, len, "%s/%s%s", dir ? dir : ".", filename, ext ? ext : "");
    return out;
}

static void xmlFree(void *ptr) {
    free(ptr);
}

static void testErrorHandler(void *ctx, const char *fmt, const char *arg) {
    (void)ctx;
    fprintf(stderr, fmt, arg);
}

/* Force the vulnerable RNG validation path to report failure (< 0) */
static int xmlTextReaderRelaxNGValidate(xmlTextReaderPtr reader, const char *rng) {
    (void)reader;
    (void)rng;
    return -1; /* simulate schema compile failure */
}

/* Remaining stubs to satisfy linker; not executed in this reproducer */
static int xmlTextReaderRead(xmlTextReaderPtr reader) { (void)reader; return 0; }
static void processNode(FILE *t, xmlTextReaderPtr reader) { (void)t; (void)reader; }
static int xmlTextReaderIsValid(xmlTextReaderPtr reader) { (void)reader; return 1; }
static int compareFiles(const char *a, const char *b) { (void)a; (void)b; return 0; }
static int compareFileMem(const char *filename, const char *mem, size_t size) {
    (void)filename; (void)mem; (void)size; return 0;
}

/* Define LIBXML_RELAXNG_ENABLED to enable the vulnerable block */
#define LIBXML_RELAXNG_ENABLED 1

/* Vulnerable function copied/replicated from runtest.c with minimal changes */
static int streamProcessTest(const char *filename, const char *result, const char *err,
                             xmlTextReaderPtr reader, const char *rng,
                             int options ATTRIBUTE_UNUSED) {
    int ret;
    char *temp = NULL;
    FILE *t = NULL;

    if (reader == NULL)
        return(-1);

    nb_tests++;
    if (result != NULL) {
        temp = resultFilename(filename, temp_directory, ".res");
        if (temp == NULL) {
            fprintf(stderr, "Out of memory\n");
            fatalError();
        }
        t = fopen(temp, "wb");
        if (t == NULL) {
            fprintf(stderr, "Can't open temp file %s\n", temp);
            xmlFree(temp);
            return(-1);
        }
    }
#ifdef LIBXML_RELAXNG_ENABLED
    if (rng != NULL) {
        ret = xmlTextReaderRelaxNGValidate(reader, rng);
        if (ret < 0) {
            testErrorHandler(NULL, "Relax-NG schema %s failed to compile\n", rng);
            /* BUG: t can be NULL here if result == NULL, leading to fclose(NULL) */
            fclose(t);
            if (temp != NULL) {
                unlink(temp);
                xmlFree(temp);
            }
            return(0);
        }
    }
#endif
    ret = xmlTextReaderRead(reader);
    while (ret == 1) {
        if ((t != NULL) && (rng == NULL))
            processNode(t, reader);
        ret = xmlTextReaderRead(reader);
    }
    if (ret != 0) {
        testErrorHandler(NULL, "%s : failed to parse\n", filename);
    }
    if (rng != NULL) {
        if (xmlTextReaderIsValid(reader) != 1) {
            testErrorHandler(NULL, "%s fails to validate\n", filename);
        } else {
            testErrorHandler(NULL, "%s validates\n", filename);
        }
    }
    if (t != NULL) {
        fclose(t);
        ret = compareFiles(temp, result);
        if (temp != NULL) {
            unlink(temp);
            xmlFree(temp);
        }
        if (ret) {
            fprintf(stderr, "Result for %s failed in %s\n", filename, result);
            return(-1);
        }
    }
    if (err != NULL) {
        ret = compareFileMem(err, testErrors, testErrorsSize);
        if (ret != 0) {
            fprintf(stderr, "Error for %s failed\n", filename);
        }
    }
    return 0;
}

int main(void) {
    /* Use any non-NULL reader pointer to satisfy the initial check */
    xmlTextReaderPtr fakeReader = (xmlTextReaderPtr)0x1;

    /* Pass result == NULL so 't' stays NULL; pass rng != NULL to take RNG path */
    const char *filename = "dummy.xml";
    const char *result = NULL;      /* ensures t is never opened */
    const char *err = NULL;
    const char *rng = "invalid.rng";/* any non-NULL string */

    /* This call will reach the bug and attempt fclose(NULL) */
    int r = streamProcessTest(filename, result, err, fakeReader, rng, 0);
    fprintf(stderr, "streamProcessTest returned %d (if no crash occurred)\n", r);
    return 0;
}
