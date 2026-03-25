#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
 * Standalone reproducer for the null-pointer-dereference in testall_dict()
 * from testdict.c: the return value of xmlMalloc for test1 is not checked
 * before passing to memset.
 */

#define NB_STRINGS_MAX 8

static char **strings1;
static char **strings2;
static char **test1;
static char **test2;

/*
 * Stub xmlMalloc that simulates an allocation failure exactly on the third
 * allocation (which corresponds to the allocation for test1 in the original
 * code path). This makes the subsequent memset(test1, ...) dereference NULL.
 */
static size_t xmlmalloc_call_count = 0;

void *xmlMalloc(size_t size) {
    xmlmalloc_call_count++;
    if (xmlmalloc_call_count == 3) {
        /* Simulate allocation failure for test1 */
        return NULL;
    }
    return malloc(size);
}

void xmlFree(void *ptr) {
    free(ptr);
}

static int testall_dict(void) {
    strings1 = (char **)xmlMalloc(NB_STRINGS_MAX * sizeof(strings1[0]));
    /* No NULL check before memset, as in the vulnerable code */
    memset(strings1, 0, NB_STRINGS_MAX * sizeof(strings1[0]));

    strings2 = (char **)xmlMalloc(NB_STRINGS_MAX * sizeof(strings2[0]));
    memset(strings2, 0, NB_STRINGS_MAX * sizeof(strings2[0]));

    test1 = (char **)xmlMalloc(NB_STRINGS_MAX * sizeof(test1[0]));
    /* Vulnerability: missing NULL check on test1 before memset */
    memset(test1, 0, NB_STRINGS_MAX * sizeof(test1[0]));

    /* Unreached due to crash above, but kept for structural similarity */
    test2 = (char **)xmlMalloc(NB_STRINGS_MAX * sizeof(test2[0]));
    memset(test2, 0, NB_STRINGS_MAX * sizeof(test2[0]));

    return 0;
}

int main(void) {
    /*
     * Trigger the bug: the third xmlMalloc (for test1) returns NULL, and
     * the following memset dereferences a NULL pointer.
     */
    (void)testall_dict();
    puts("Unexpectedly survived");
    return 0;
}
