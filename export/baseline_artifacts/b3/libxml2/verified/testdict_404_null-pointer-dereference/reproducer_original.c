#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * Self-contained reproducer for the null-pointer-dereference in testdict.c:testall_dict
 * It mimics the vulnerable allocation + memset sequence where xmlMalloc can
 * return NULL and its result is passed directly to memset, causing a crash.
 */

/* Minimal stand-ins for libxml2-ish types/APIs used by the test code */
typedef void* xmlDictPtr;

/* Global arrays as in the original test code */
static char **strings1 = NULL;
static char **strings2 = NULL;
static char **test1 = NULL;
static char **test2 = NULL;

/* Constants used by the test code */
#define NB_STRINGS_MAX 1024

/* Stub implementations for functions referenced later but not reached */
static void fill_string_pool(char **unused1, unsigned unused2) { (void)unused1; (void)unused2; }
static void clean_strings(void) {}
static int test_dict(xmlDictPtr dict) { (void)dict; return 0; }
static int test_subdict(xmlDictPtr dict) { (void)dict; return 0; }
static xmlDictPtr xmlDictCreate(void) { return (xmlDictPtr)0x1; }
static void xmlDictFree(xmlDictPtr dict) { (void)dict; }

/* Stub xmlFree compatible with our stubs */
static void xmlFree(void *ptr) { free(ptr); }

/* Critical piece: xmlMalloc that simulates allocation failure (returns NULL) */
static void *xmlMalloc(size_t size) {
    (void)size; /* Simulate OOM regardless of requested size */
    return NULL;
}

/* Function replicating the vulnerable logic around the reported crash site */
static int testall_dict(void) {
    xmlDictPtr dict;
    int ret = 0;

    /* xmlMalloc returns NULL by design; memset on NULL triggers the bug */
    strings1 = (char **)xmlMalloc(NB_STRINGS_MAX * sizeof(strings1[0]));
    /* Vulnerable line: no NULL check before memset */
    memset(strings1, 0, NB_STRINGS_MAX * sizeof(strings1[0]));

    /* The following lines are never reached due to the crash above,
       but are kept to mirror structure of the original function. */
    strings2 = (char **)xmlMalloc(NB_STRINGS_MAX * sizeof(strings2[0]));
    memset(strings2, 0, NB_STRINGS_MAX * sizeof(strings2[0]));
    test1 = (char **)xmlMalloc(NB_STRINGS_MAX * sizeof(test1[0]));
    memset(test1, 0, NB_STRINGS_MAX * sizeof(test1[0]));
    test2 = (char **)xmlMalloc(NB_STRINGS_MAX * sizeof(test2[0]));
    memset(test2, 0, NB_STRINGS_MAX * sizeof(test2[0]));

    fill_string_pool(strings1, 0);
    fill_string_pool(strings2, 0);

    dict = xmlDictCreate();
    if (dict == NULL) {
        fprintf(stderr, "Out of memory while creating dictionary\n");
        exit(1);
    }
    if (test_dict(dict) != 0)
        ret = 1;
    if (test_subdict(dict) != 0)
        ret = 1;
    xmlDictFree(dict);

    clean_strings();
    xmlFree(strings1);
    xmlFree(strings2);
    xmlFree(test1);
    xmlFree(test2);

    return ret;
}

int main(void) {
    /* Trigger the vulnerable path */
    return testall_dict();
}
