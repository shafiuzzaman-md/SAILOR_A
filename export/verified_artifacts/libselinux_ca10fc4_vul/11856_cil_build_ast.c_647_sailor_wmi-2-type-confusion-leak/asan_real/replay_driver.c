#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

int cil_gen_perm(struct cil_db *db, struct cil_tree_node *parse_current, struct cil_tree_node *ast_node, enum cil_flavor flavor, unsigned int *num_perms);

int main() {
    // Concrete allocations per model
    struct cil_db *db = calloc(1, sizeof(*db));
    struct cil_tree_node *parse_current = calloc(1, sizeof(*parse_current));
    struct cil_tree_node *ast_node = calloc(1, sizeof(*ast_node));
    unsigned int num_perms = 0;

    // Prepare a buffer for parse_current->data (treated as key)
    char *keybuf = malloc(32);
    { static const unsigned char keybuf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(keybuf, keybuf_data, (32 < sizeof(keybuf_data)) ? 32 : sizeof(keybuf_data)); };
    keybuf[31] = '\0';
    parse_current->data = keybuf;

    // Make flavor symbolic
    enum cil_flavor flavor;
    { static const unsigned char flavor_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&flavor, flavor_data, (sizeof(flavor) < sizeof(flavor_data)) ? sizeof(flavor) : sizeof(flavor_data)); };

    // UAF setup: free the parse_current object but keep the stale pointer value
    free(parse_current);

    // Call entry: cil_gen_perm will dereference a freed pointer (UAF)
    cil_gen_perm(db, parse_current, ast_node, flavor, &num_perms);
    return 0;
}
