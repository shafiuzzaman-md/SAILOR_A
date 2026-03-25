#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int cil_complex_symtab_insert(struct cil_complex_symtab *symtab,
                  struct cil_complex_symtab_key *ckey,
                  struct cil_complex_symtab_datum *datum);

int main() {
    // Allocate symtab with a single bucket (hash forced to 0 in harness)
    struct cil_complex_symtab *symtab = (struct cil_complex_symtab *)calloc(1, sizeof(*symtab));
    if (!symtab) return 0;
    symtab->nslots = 1;
    symtab->mask = 0;  // matches harness hash implementation
    symtab->htable = (struct cil_complex_symtab_node **)calloc(symtab->nslots, sizeof(*symtab->htable));

    // Create a node, place its pointer in the table, then free it to simulate stale reference
    struct cil_complex_symtab_node *stale = (struct cil_complex_symtab_node *)malloc(sizeof(*stale));
    if (!stale) return 0;
    memset(stale, 0, sizeof(*stale));

    // Give the stale node a key so comparisons don't segfault if reached
    struct cil_complex_symtab_key *stale_key = (struct cil_complex_symtab_key *)calloc(1, sizeof(*stale_key));
    if (!stale_key) return 0;
    { static const unsigned char stale_key_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(stale_key, stale_key_data, (sizeof(*stale_key) < sizeof(stale_key_data)) ? sizeof(*stale_key) : sizeof(stale_key_data)); };
    stale->ckey = stale_key;
    stale->next = NULL;  // single element list

    symtab->htable[0] = stale;   // store pointer into table
    free(stale);                 // free the node -> symtab holds stale pointer

    // Prepare inputs for insert
    struct cil_complex_symtab_key *ckey = (struct cil_complex_symtab_key *)calloc(1, sizeof(*ckey));
    struct cil_complex_symtab_datum *datum = (struct cil_complex_symtab_datum *)calloc(1, sizeof(*datum));
    if (!ckey || !datum) return 0;
    { static const unsigned char ckey_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(ckey, ckey_data, (sizeof(*ckey) < sizeof(ckey_data)) ? sizeof(*ckey) : sizeof(ckey_data)); };

    // Direct call to entry (which directly calls the vulnerable function)
    cil_complex_symtab_insert(symtab, ckey, datum);
    return 0;
}
