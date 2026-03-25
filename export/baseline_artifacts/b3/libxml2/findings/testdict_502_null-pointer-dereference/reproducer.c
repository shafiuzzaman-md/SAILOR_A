#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Minimal libxml2-compatible typedefs/stubs */
typedef unsigned char xmlChar;

/* Stub allocator that simulates allocation failure to trigger the bug */
void *xmlMalloc(size_t size) {
    (void)size;
    return NULL; /* Force failure on every allocation */
}

void xmlFree(void *ptr) {
    /* Free is a no-op for this reproducer; free(NULL) is safe anyway */
    free(ptr);
}

/* Copied/derived structure and vulnerable function from testdict.c */
typedef struct {
    xmlChar **strings;
    size_t num_entries;
    size_t num_keys;
    size_t num_strings;
    size_t index;
    xmlChar id;
} StringPool;

static StringPool *
pool_new(size_t num_entries, size_t num_keys, xmlChar id) {
    StringPool *ret;
    size_t num_strings;

    ret = (StringPool *)xmlMalloc(sizeof(*ret));
    /* Vulnerability: ret is dereferenced without a NULL check */
    ret->num_entries = num_entries; /* NULL deref here when xmlMalloc fails */
    ret->num_keys = num_keys;
    num_strings = num_entries * num_keys;
    ret->strings = (xmlChar **)xmlMalloc(num_strings * sizeof(ret->strings[0]));
    memset(ret->strings, 0, num_strings * sizeof(ret->strings[0]));
    ret->num_strings = num_strings;
    ret->index = 0;
    ret->id = id;

    return ret;
}

int main(void) {
    /* Any arguments will do; xmlMalloc is forced to fail */
    (void)pool_new(10, 10, (xmlChar)'X');

    /* If the program reaches this point, the bug did not trigger as expected */
    puts("Unexpectedly survived null dereference");
    return 0;
}
