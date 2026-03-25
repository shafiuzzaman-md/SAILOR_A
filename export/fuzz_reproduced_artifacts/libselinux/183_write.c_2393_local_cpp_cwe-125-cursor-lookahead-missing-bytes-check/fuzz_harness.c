#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

int policydb_write(struct policydb *p, struct policy_file *fp);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 12) return 0;
    struct policydb *p = (struct policydb *)calloc(1, sizeof(struct policydb));
    struct policy_file *fp = (struct policy_file *)calloc(1, sizeof(struct policy_file));

    // Overapproximate: make struct contents symbolic (sizes are concrete)
    if (p) { memcpy(p, fuzz_data + 0, 4); };
    if (fp) { memcpy(fp, fuzz_data + 4, 8); };

    // Direct call to entry (no guards)
    policydb_write(p, fp);
    return 0;
}
