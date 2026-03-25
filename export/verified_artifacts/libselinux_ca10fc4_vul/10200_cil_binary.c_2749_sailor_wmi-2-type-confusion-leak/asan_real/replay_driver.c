#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Entry declared in harness
int cil_roleallow_to_policydb(policydb_t *pdb, const struct cil_db *db, struct cil_roleallow *roleallow);

int main() {
    policydb_t *pdb = NULL;
    const struct cil_db *db = NULL;

    // Allocate cil_roleallow concretely
    struct cil_roleallow *ra = (struct cil_roleallow *)malloc(sizeof(struct cil_roleallow));
    if (!ra) return 0;
    memset(ra, 0, sizeof(*ra));

    // Backing objects for src/tgt
    void *src_obj = malloc(32);
    void *tgt_obj = malloc(32);
    if (!src_obj || !tgt_obj) return 0;

    // Make contents symbolic for over-approximation
    { static const unsigned char src_obj_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(src_obj, src_obj_bytes_data, (32 < sizeof(src_obj_bytes_data)) ? 32 : sizeof(src_obj_bytes_data)); };
    { static const unsigned char tgt_obj_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(tgt_obj, tgt_obj_bytes_data, (32 < sizeof(tgt_obj_bytes_data)) ? 32 : sizeof(tgt_obj_bytes_data)); };

    // Initialize roleallow fields
    ra->src = src_obj;
    ra->tgt = tgt_obj;

    // UAF pattern: free the cil_roleallow container, then pass the stale pointer
    free(ra);

    // Call entry with the freed pointer; accessing ra->tgt in harness triggers .free.err
    (void)cil_roleallow_to_policydb(pdb, db, ra);
    return 0;
}
