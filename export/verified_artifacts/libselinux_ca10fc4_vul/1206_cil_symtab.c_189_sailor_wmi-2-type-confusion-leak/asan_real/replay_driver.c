#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

// Prototypes from harness
int cil_complex_symtab_insert(struct cil_complex_symtab *symtab,
               struct cil_complex_symtab_key *ckey,
               void *datum);
void cil_complex_symtab_destroy(struct cil_complex_symtab *symtab);

int main() {
    // Allocate a symtab with 1 bucket (mask=0 => hash always 0)
    struct cil_complex_symtab *sym = calloc(1, sizeof(*sym));
    sym->mask = 0; // one bucket
    sym->htable = calloc(1, sizeof(*sym->htable) * (sym->mask + 1));

    // Create a node and insert it into bucket 0
    struct cil_complex_symtab_node *node = calloc(1, sizeof(*node));
    struct cil_complex_symtab_key *oldkey = calloc(1, sizeof(*oldkey));
    // Make contents symbolic (values don't matter for UAF; the deref itself is the bug)
    { static const unsigned char oldkey_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(oldkey, oldkey_data, (sizeof(*oldkey) < sizeof(oldkey_data)) ? sizeof(*oldkey) : sizeof(oldkey_data)); };
    node->ckey = oldkey;
    node->next = NULL;
    sym->htable[0] = node;

    // Destroy frees node and its key but leaves stale pointer in htable[0]
    cil_complex_symtab_destroy(sym);

    // Prepare a new key for insertion (symbolic to overapproximate)
    struct cil_complex_symtab_key *newkey = calloc(1, sizeof(*newkey));
    { static const unsigned char newkey_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(newkey, newkey_data, (sizeof(*newkey) < sizeof(newkey_data)) ? sizeof(*newkey) : sizeof(newkey_data)); };

    // Datum can be any pointer
    void *datum = malloc(8);

    // Call entry function: this will traverse the bucket and dereference the freed node
    cil_complex_symtab_insert(sym, newkey, datum);

    return 0;
}
