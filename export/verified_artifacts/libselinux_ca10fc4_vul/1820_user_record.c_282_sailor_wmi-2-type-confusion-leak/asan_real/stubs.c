#include "harness_types.h"
#include <stddef.h>
#include <stdlib.h>

// Force allocation failure in target path so tmp_roles == NULL
void *calloc(size_t nmemb, size_t size) {
    (void)nmemb; (void)size;
    return NULL;
}
