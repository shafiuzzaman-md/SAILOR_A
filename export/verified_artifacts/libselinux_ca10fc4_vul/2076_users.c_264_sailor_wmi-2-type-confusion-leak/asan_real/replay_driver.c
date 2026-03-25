#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

// entry_func is defined in harness/users.c
int sepol_user_modify(sepol_handle *handle);

int main() {
    // Allocate a real sepol_handle object
    sepol_handle *h = (sepol_handle *)malloc(sizeof(sepol_handle));
    if (!h) return 0;

    // Make its contents symbolic (attacker-controlled)
    { static const unsigned char sepol_handle_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(h, sepol_handle_bytes_data, (sizeof(*h) < sizeof(sepol_handle_bytes_data)) ? sizeof(*h) : sizeof(sepol_handle_bytes_data)); };

    // Model the WMI-2 stale reference: free the object but keep a pointer to it
    free(h);

    // Use-after-free: pass the freed pointer to the entry function
    sepol_user_modify(h);
    return 0;
}
