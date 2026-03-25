// Standalone C reproducer for null-pointer-dereference in pool_new memset
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Minimal typedef to match libxml2 style
typedef unsigned char xmlChar;

// Global counter to control simulated allocation failures
static int g_xmlmalloc_calls = 0;

// Stub implementations mimicking libxml2 allocation API
void *xmlMalloc(size_t size) {
    g_xmlmalloc_calls++;
    // First allocation (for StringPool struct) succeeds
    if (g_xmlmalloc_calls == 1) {
        return malloc(size);
    }
    // Second allocation (for ret->strings) fails to trigger the bug
    if (g_xmlmalloc_calls == 2) {
        return NULL;
    }
    // Any further allocations succeed (not used here)
    return malloc(size);
}

void xmlFree(void *ptr) {
    free(ptr);
}

// Mirror of the vulnerable code context
typedef struct {
    xmlChar **strings;
    size_t num_entries;
    size_t num_keys;
    size_t num_strings;
    size_t index;
    xmlChar id;
} StringPool;

// Vulnerable function copied from the context
static StringPool *
pool_new(size_t num_entries, size_t num_keys, xmlChar id) {
    StringPool *ret;
    size_t num_strings;

    ret = (StringPool *)xmlMalloc(sizeof(*ret));
    // Intentionally no NULL check to mirror original behavior
    ret->num_entries = num_entries;
    ret->num_keys = num_keys;
    num_strings = num_entries * num_keys;
    ret->strings = (xmlChar **)xmlMalloc(num_strings * sizeof(ret->strings[0]));
    // Vulnerable memset: ret->strings may be NULL, size > 0
    memset(ret->strings, 0, num_strings * sizeof(ret->strings[0]));
    ret->num_strings = num_strings;
    ret->index = 0;
    ret->id = id;

    return ret;
}

int main(void) {
    // Choose values so num_strings > 0, ensuring memset size > 0
    size_t num_entries = 10;
    size_t num_keys = 10;
    xmlChar id = (xmlChar)'x';

    // This call will crash inside pool_new at the memset when ret->strings == NULL
    (void)pool_new(num_entries, num_keys, id);

    // Should never reach here
    printf("Unexpectedly succeeded.\n");
    return 0;
}
