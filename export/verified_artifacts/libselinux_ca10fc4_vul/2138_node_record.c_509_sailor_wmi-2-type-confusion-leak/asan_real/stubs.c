// NO_HARNESS_TYPES
// klee removed
#include <stddef.h>

void *malloc(size_t size) {
    (void)size;
    return 0; // force failure to take ERR path
}

void free(void *p) {
    (void)p; // no-op
}
