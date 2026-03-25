#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int cil_typealias_to_policydb(policydb_t *pdb, struct cil_alias *cil_alias);

int main() {
    // Concrete allocations
    policydb_t *pdb = (policydb_t *)calloc(1, sizeof(*pdb));
    struct cil_alias *alias = (struct cil_alias *)calloc(1, sizeof(*alias));

    // Allocate and wire concrete pointers used along the path
    alias->actual = (struct cil_symtab_datum *)calloc(1, sizeof(*alias->actual));

    // Symbolic string content with concrete size
    char *fqn = (char *)malloc(16);
    { static const unsigned char alias_fqn_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(fqn, alias_fqn_data, (16 < sizeof(alias_fqn_data)) ? 16 : sizeof(alias_fqn_data)); };
    fqn[15] = '\0';
    alias->datum.fqn = fqn;

    // Optionally make pdb contents symbolic (not dereferenced in slice)
    { static const unsigned char pdb_sym_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(pdb, pdb_sym_data, (sizeof(*pdb) < sizeof(pdb_sym_data)) ? sizeof(*pdb) : sizeof(pdb_sym_data)); };

    // Do NOT make 'alias' itself symbolic to avoid corrupting its pointers

    // Call entry (pure pass-through)
    cil_typealias_to_policydb(pdb, alias);
    return 0;
}
