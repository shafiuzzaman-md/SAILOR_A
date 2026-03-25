#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Self-contained reproducer for the null-pointer-dereference in testall_dict
 * The bug: result of xmlMalloc for test2 is not checked before memset.
 * We stub xmlMalloc to return NULL on the 4th allocation to hit the bug.
 */

#define NB_STRINGS_MAX 64
#define NB_STRINGS_NS 8

/* Global arrays as used by testdict.c */
static void **strings1 = NULL;
static void **strings2 = NULL;
static void **test1    = NULL;
static void **test2    = NULL;

/* Dummy seeds referenced by fill_string_pool calls later in the function */
static void *seeds1 = NULL;
static void *seeds2 = NULL;

/* Minimal xmlDict types and stubs so we can link */
typedef void* xmlDictPtr;

static xmlDictPtr xmlDictCreate(void) {
    /* not reached due to earlier crash */
    return (xmlDictPtr)0x1;
}

static void xmlDictFree(xmlDictPtr dict) {
    (void)dict;
}

/* Stubs for other referenced functions (not reached) */
static void fill_string_pool(void **pool, void *seeds) {
    (void)pool; (void)seeds;
}

static void print_strings(void) { }
static void clean_strings(void) { }
static int test_dict(xmlDictPtr dict) { (void)dict; return 0; }
static int test_subdict(xmlDictPtr dict) { (void)dict; return 0; }

/* xmlMalloc/xmlFree stubs. Force failure on 4th allocation (test2). */
static int xmlmalloc_call_count = 0;
void *xmlMalloc(size_t size) {
    xmlmalloc_call_count++;
    if (xmlmalloc_call_count == 4) {
        /* Simulate allocation failure for test2 */
        return NULL;
    }
    return malloc(size);
}

void xmlFree(void *ptr) {
    free(ptr);
}

/* Vulnerable function replicated from the snippet */
static int testall_dict(void) {
    xmlDictPtr dict;
    int ret = 0;

    strings1 = xmlMalloc(NB_STRINGS_MAX * sizeof(strings1[0]));
    memset(strings1, 0, NB_STRINGS_MAX * sizeof(strings1[0]));
    strings2 = xmlMalloc(NB_STRINGS_MAX * sizeof(strings2[0]));
    memset(strings2, 0, NB_STRINGS_MAX * sizeof(strings2[0]));
    test1 = xmlMalloc(NB_STRINGS_MAX * sizeof(test1[0]));
    memset(test1, 0, NB_STRINGS_MAX * sizeof(test1[0]));
    test2 = xmlMalloc(NB_STRINGS_MAX * sizeof(test2[0]));
    /* BUG: test2 is not checked for NULL before memset, will crash here */
    memset(test2, 0, NB_STRINGS_MAX * sizeof(test2[0]));

    /* The following code is not reached due to the crash above */
    fill_string_pool(strings1, seeds1);
    fill_string_pool(strings2, seeds2);

    dict = xmlDictCreate();
    if (dict == NULL) {
        fprintf(stderr, "Out of memory while creating dictionary\n");
        exit(1);
    }
    if (test_dict(dict) != 0) {
        ret = 1;
    }
    if (test_subdict(dict) != 0) {
        ret = 1;
    }
    xmlDictFree(dict);

    clean_strings();
    xmlFree(strings1);
    xmlFree(strings2);
    xmlFree(test1);
    xmlFree(test2);

    return ret;
}

int main(void) {
    /* Trigger the vulnerability */
    (void)testall_dict();
    return 0;
}