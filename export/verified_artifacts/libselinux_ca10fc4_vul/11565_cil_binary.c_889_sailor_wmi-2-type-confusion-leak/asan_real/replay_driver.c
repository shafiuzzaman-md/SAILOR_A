#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int cil_sensitivityorder_to_policydb(policydb_t *pdb, const struct cil_db *db);

int main() {
    // Allocate policydb_t (unused in the path, but pass a valid pointer)
    policydb_t *pdb = (policydb_t *)calloc(1, sizeof(policydb_t));

    // Phase 1: allocate cil_db concretely
    struct cil_db *db = (struct cil_db *)calloc(1, sizeof(struct cil_db));
    // Make db content symbolic to overapproximate state
    { static const unsigned char cil_db_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(db, cil_db_bytes_data, (sizeof(*db) < sizeof(cil_db_bytes_data)) ? sizeof(*db) : sizeof(cil_db_bytes_data)); };

    // Keep a stale alias to simulate lingering reference after destroy
    const struct cil_db *stale_db = db;

    // Simulate cil_db_destroy() that frees the db (WMI-2 pattern setup)
    free(db);

    // Phase 2: use-after-free via stale_db in cil_sensitivityorder_to_policydb()
    cil_sensitivityorder_to_policydb(pdb, stale_db);

    return 0;
}
