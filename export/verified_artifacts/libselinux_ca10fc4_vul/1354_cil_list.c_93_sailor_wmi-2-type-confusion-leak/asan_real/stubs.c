#include "harness_types.h"
// klee removed
#include <stdlib.h>

void cil_destroy_data(void **data, enum cil_flavor flavor) {
    (void)data;
    (void)flavor;
}
