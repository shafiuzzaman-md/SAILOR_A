#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

// entry from harness
int cil_type_to_policydb(struct policydb *pdb, struct cil_type *cil_type, void *type_value_to_cil[]);

typedef struct policydb policydb_t; // match harness forward decl

int main() {
    // pdb is unused in the harness; pass NULL
    policydb_t *pdb = NULL;

    // Allocate cil_type concretely
    struct cil_type *t = (struct cil_type *)malloc(sizeof(*t));
    if (!t) return 0;
    memset(t, 0, sizeof(*t));

    // Assign a concrete fqn buffer (not necessary for free.err, but harmless)
    char *fqn = (char *)malloc(16);
    if (!fqn) return 0;
    // Make content symbolic to overapproximate
    { static const unsigned char fqn_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(fqn, fqn_data, (16 < sizeof(fqn_data)) ? 16 : sizeof(fqn_data)); };
    // Ensure NUL-termination somewhere to keep strdup safe if reached
    fqn[15] = '\0';
    t->datum.fqn = fqn;

    // WMI-2/UAF setup: free the cil_type object, leaving a stale pointer
    free(t);

    // Prepare third argument (unused in harness)
    void *type_value_to_cil[8] = {0};

    // Call pass-through entry; this will read from freed memory -> KLEE .free.err
    // NOTE: Passing the stale pointer 't' post-free on purpose
    cil_type_to_policydb(pdb, (struct cil_type *)t, type_value_to_cil);

    return 0;
}
