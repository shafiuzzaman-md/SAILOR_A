#include <string.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// Minimal local type defs to match the call site
typedef struct { int dummy; } sepol_module_package_t;
struct sepol_handle { int dummy; };
struct sepol_policy_file_pf { struct sepol_handle *handle; };
struct sepol_policy_file { struct sepol_policy_file_pf pf; };

// match 2-arg signature
int sepol_module_package_read(sepol_module_package_t *mod, struct sepol_policy_file *spf);


int main() {
    // Concrete allocations per instructions
    sepol_module_package_t *mod = (sepol_module_package_t *)calloc(1, sizeof(sepol_module_package_t));
    struct sepol_policy_file *spf = (struct sepol_policy_file *)calloc(1, sizeof(struct sepol_policy_file));
    struct sepol_handle *h = (struct sepol_handle *)malloc(sizeof(struct sepol_handle));

    // Fill handle bytes symbolically, then free to create a stale pointer
    { static const unsigned char sepol_handle_bytes_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(h, sepol_handle_bytes_data, (sizeof(*h) < sizeof(sepol_handle_bytes_data)) ? sizeof(*h) : sizeof(sepol_handle_bytes_data)); };
    spf->pf.handle = h;   // store pointer into policy file

    // Free the handle to create UAF; do NOT nullify spf->pf.handle
    free(h);

    // Direct call to the vulnerable/entry function
    sepol_module_package_read(mod, spf);

    return 0;
}
