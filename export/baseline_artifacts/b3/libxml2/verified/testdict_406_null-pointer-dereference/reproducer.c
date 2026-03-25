#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal stand-ins for libxml2 types/APIs used in the vulnerable function */
typedef struct _xmlDict { int dummy; } xmlDict;
typedef xmlDict* xmlDictPtr;

xmlDictPtr xmlDictCreate(void) { return (xmlDictPtr)malloc(sizeof(xmlDict)); }
void xmlDictFree(xmlDictPtr d) { free(d); }

/* Stubs to satisfy references from testall_dict (won't be reached before crash) */
int test_dict(xmlDictPtr dict) { (void)dict; return 0; }
int test_subdict(xmlDictPtr dict) { (void)dict; return 0; }
void fill_string_pool(char **arr, unsigned int *seeds) { (void)arr; (void)seeds; }
void clean_strings(void) {}

/* Globals mimicking the test harness */
#define NB_STRINGS_MAX 16

static char **strings1;
static char **strings2;
static char **test1;
static char **test2;

unsigned int seeds1[1] = {0};
unsigned int seeds2[1] = {0};

/* Override xmlMalloc/xmlFree to control allocation behavior */
static int xmlmalloc_calls = 0;
void *xmlMalloc(size_t size) {
    xmlmalloc_calls++;
    /* 1st call: strings1, 2nd call: strings2 (we force NULL on 2nd) */
    if (xmlmalloc_calls == 2) {
        return NULL; /* Simulate OOM for strings2 */
    }
    return malloc(size);
}
void xmlFree(void *ptr) { free(ptr); }

/* Reimplementation of the vulnerable function around the reported lines */
static int testall_dict(void) {
    xmlDictPtr dict;
    int ret = 0;

    strings1 = xmlMalloc(NB_STRINGS_MAX * sizeof(strings1[0]));
    memset(strings1, 0, NB_STRINGS_MAX * sizeof(strings1[0]));
    strings2 = xmlMalloc(NB_STRINGS_MAX * sizeof(strings2[0]));
    /* Vulnerable line: strings2 may be NULL here, memset will dereference NULL */
    memset(strings2, 0, NB_STRINGS_MAX * sizeof(strings2[0]));

    /* The rest is never reached due to the crash above, but kept for fidelity */
    test1 = xmlMalloc(NB_STRINGS_MAX * sizeof(test1[0]));
    memset(test1, 0, NB_STRINGS_MAX * sizeof(test1[0]));
    test2 = xmlMalloc(NB_STRINGS_MAX * sizeof(test2[0]));
    memset(test2, 0, NB_STRINGS_MAX * sizeof(test2[0]));

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
    /* Running this will crash at the vulnerable memset(strings2, ...) */
    (void)testall_dict();
    return 0;
}