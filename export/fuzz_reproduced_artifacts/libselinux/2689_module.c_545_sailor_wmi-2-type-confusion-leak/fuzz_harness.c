#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

// Prototype from harness
int sepol_module_package_read(sepol_module_package_t *mod, struct sepol_policy_file *spf, int verbose);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 16) return 0;
    // Allocate concrete containers
    sepol_module_package_t *mod = (sepol_module_package_t *)calloc(1, sizeof(sepol_module_package_t));
    struct sepol_policy_file *spf = (struct sepol_policy_file *)calloc(1, sizeof(struct sepol_policy_file));
    struct sepol_handle *h = (struct sepol_handle *)calloc(1, sizeof(struct sepol_handle));

    // Make handle fields symbolic (contents attacker-controlled)
    { memcpy(h, fuzz_data + 0, 16); };

    // Assign handle into policy file, then free it to create a stale reference (UAF setup)
    spf->pf.handle = h;
    free(h); // do NOT nullify spf->pf.handle

    // Verbose can be symbolic; not relevant to path here
    int verbose;
    { static const unsigned char verbose_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&verbose, verbose_data, (sizeof(verbose) < sizeof(verbose_data)) ? sizeof(verbose) : sizeof(verbose_data)); };

    // Call entry which will lead to ERR(file->handle, ...) and deref the freed handle
    sepol_module_package_read(mod, spf, verbose);
    return 0;
}
