// NO_HARNESS_TYPES
#include <stddef.h>
#include <stdlib.h>

// Always succeed and allocate a small fixed buffer to avoid symbolic-size malloc
void *klee_malloc_fail(size_t size) {
    (void)size;
    // fixed small buffer; if caller copies more, it will overflow and KLEE will catch it
    return malloc(64);
}
