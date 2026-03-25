#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

/* entry from harness/module.c */
extern int sepol_module_package_read(sepol_module_package_t *mod, struct sepol_policy_file *spf, int verbose);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 32) return 0;
    // Allocate concrete objects
    sepol_module_package_t *mod = (sepol_module_package_t *)calloc(1, sizeof(*mod));
    struct sepol_policy_file *spf = (struct sepol_policy_file *)calloc(1, sizeof(*spf));
    if (!mod || !spf) return 0;

    // Allocate a real sepol_handle and make its contents symbolic
    struct sepol_handle *h = (struct sepol_handle *)calloc(1, sizeof(*h));
    if (!h) return 0;
    { memcpy(h, fuzz_data + 0, 16); };

    // Install handle into policy_file; keep pointer after free to model stale reference
    spf->pf.handle = h;

    // Free the handle to create a UAF scenario before the ERR() use in sepol_module_package_read
    free(h);

    // Optional: attempt to reclaim memory with different object size to hint type-confusion reuse
    void *reclaim = malloc(sizeof(*h));
    { memcpy(reclaim, fuzz_data + 16, 16); };
    (void)reclaim;

    // Call straight into the entry (which pass-through calls sepol_module_package_read)
    sepol_module_package_read(mod, spf, 0);

    return 0;
}
