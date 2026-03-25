#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 16) return 0;
    role_datum_t *x = (role_datum_t *)calloc(1, sizeof(role_datum_t));
    if (!x) return 0;

    // Overapproximate contents
    { memcpy(x, fuzz_data + 0, 16); };

    // WMI-2/UAF pattern: free the object, then use it inside role_datum_init
    free(x);

    // Use-after-free: role_datum_init will touch x->types via type_set_init
    role_datum_init(x);
    return 0;
}
