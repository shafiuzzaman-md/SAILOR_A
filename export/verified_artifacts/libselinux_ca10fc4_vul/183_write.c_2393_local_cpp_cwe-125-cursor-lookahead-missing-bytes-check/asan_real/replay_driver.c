#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
// klee removed for replay

int policydb_write(struct policydb *p, struct policy_file *fp);

int main() {
    struct policydb *p = (struct policydb *)calloc(1, sizeof(struct policydb));
    struct policy_file *fp = (struct policy_file *)calloc(1, sizeof(struct policy_file));

    // Overapproximate: make struct contents symbolic (sizes are concrete)
    if (p) { static const unsigned char policydb_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(p, policydb_data, (sizeof(*p) < sizeof(policydb_data)) ? sizeof(*p) : sizeof(policydb_data)); };
    if (fp) { static const unsigned char policy_file_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(fp, policy_file_data, (sizeof(*fp) < sizeof(policy_file_data)) ? sizeof(*fp) : sizeof(policy_file_data)); };

    // Direct call to entry (no guards)
    policydb_write(p, fp);
    return 0;
}
