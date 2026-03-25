#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Prototype of harness entry
int cil_complex_symtab_insert(struct cil_complex_symtab *symtab,
                  struct cil_complex_symtab_key *ckey,
                  struct cil_complex_symtab_datum *datum);

int main() {
    // Allocate and initialize symtab with a single bucket (hash=0 in harness)
    struct cil_complex_symtab *symtab = (struct cil_complex_symtab *)calloc(1, sizeof(*symtab));
    symtab->nslots = 1;
    symtab->mask = 0; // not used by harness hash
    symtab->htable = (struct cil_complex_symtab_node **)calloc(symtab->nslots, sizeof(*symtab->htable));

    // Create a node, make it symbolic, place in htable[0], then free to create stale pointer
    struct cil_complex_symtab_node *stale = (struct cil_complex_symtab_node *)malloc(sizeof(*stale));
    { static const unsigned char stale_node_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(stale, stale_node_data, (sizeof(*stale) < sizeof(stale_node_data)) ? sizeof(*stale) : sizeof(stale_node_data)); };
    symtab->htable[0] = stale;   // store pointer into table
    free(stale);                  // now symtab holds a stale pointer (UAF on access)

    // Prepare insertion key and datum
    struct cil_complex_symtab_key *ckey = (struct cil_complex_symtab_key *)malloc(sizeof(*ckey));
    { static const unsigned char ckey_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(ckey, ckey_data, (sizeof(*ckey) < sizeof(ckey_data)) ? sizeof(*ckey) : sizeof(ckey_data)); };

    struct cil_complex_symtab_datum *datum = (struct cil_complex_symtab_datum *)malloc(sizeof(*datum));
    { static const unsigned char datum_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(datum, datum_data, (sizeof(*datum) < sizeof(datum_data)) ? sizeof(*datum) : sizeof(datum_data)); };

    // Direct call to entry (no guards)
    cil_complex_symtab_insert(symtab, ckey, datum);
    return 0;
}
