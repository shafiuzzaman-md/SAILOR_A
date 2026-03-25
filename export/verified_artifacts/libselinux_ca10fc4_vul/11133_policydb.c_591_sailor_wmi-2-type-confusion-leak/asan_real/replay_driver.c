#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>

int main(void) {
    role_datum_t *x = (role_datum_t *)calloc(1, sizeof(role_datum_t));
    if (!x) return 0;

    // Overapproximate contents
    { static const unsigned char role_datum_x_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(x, role_datum_x_data, (sizeof(*x) < sizeof(role_datum_x_data)) ? sizeof(*x) : sizeof(role_datum_x_data)); };

    // WMI-2/UAF pattern: free the object, then use it inside role_datum_init
    free(x);

    // Use-after-free: role_datum_init will touch x->types via type_set_init
    role_datum_init(x);
    return 0;
}
