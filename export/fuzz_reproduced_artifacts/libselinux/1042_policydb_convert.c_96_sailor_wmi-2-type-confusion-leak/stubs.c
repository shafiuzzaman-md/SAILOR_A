#include "harness_types.h"
#include <stdlib.h>
// klee removed

void sepol_handle_destroy(sepol_handle_t *handle) {
    // Real implementation frees the handle; do the same to model UAF/WMI-2
    free(handle);
}
