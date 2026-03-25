#include "harness_types.h"
#include <stddef.h>

// KLEE-visible strncpy model: copies exactly n bytes without bounds checks
// This lets KLEE detect out-of-bounds writes on dest.
char *strncpy(char *dest, const char *src, size_t n) {
    (void)src; (void)n; return dest;
}
