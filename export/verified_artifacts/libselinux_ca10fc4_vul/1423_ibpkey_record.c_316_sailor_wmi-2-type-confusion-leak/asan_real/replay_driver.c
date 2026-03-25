#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

int sepol_ibpkey_clone(sepol_handle_t *handle, const sepol_ibpkey_t *in_ibpkey);

int main() {
    // Allocate an ibpkey object concretely
    sepol_ibpkey_t *ibp = (sepol_ibpkey_t *)malloc(sizeof(sepol_ibpkey_t));
    if (!ibp) return 0;

    // Make its contents symbolic to overapproximate attacker control
    { static const unsigned char ibp_sym_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(ibp, ibp_sym_data, (sizeof(*ibp) < sizeof(ibp_sym_data)) ? sizeof(*ibp) : sizeof(ibp_sym_data)); };

    // Avoid taking the sepol_context_clone path (not needed for the vulnerability)
    ibp->con = NULL;

    // Free the object to create a dangling pointer (UAF scenario)
    free(ibp);

    // Pass the freed pointer into the entry function, which will dereference it
    sepol_ibpkey_clone(NULL, ibp);

    return 0;
}
