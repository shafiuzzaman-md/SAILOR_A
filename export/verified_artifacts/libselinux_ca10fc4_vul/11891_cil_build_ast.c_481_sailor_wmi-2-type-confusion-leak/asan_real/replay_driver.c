#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// entry_func is defined in harness/cil_build_ast.c
extern int cil_gen_blockabstract(struct cil_db *db, struct cil_tree_node *parse_current, struct cil_tree_node *ast_node);

int main() {
    // Allocate db, parse_current, ast_node concretely
    struct cil_db *db = (struct cil_db *)calloc(1, sizeof(struct cil_db));
    struct cil_tree_node *parse_current = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *ast_node = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));

    // Create a next node, set fields, then free it to create a dangling pointer
    struct cil_tree_node *next = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));

    // Give next->data some backing memory (symbolic) before free to emulate realistic state
    char *payload = (char *)malloc(64);
    { static const unsigned char next_payload_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(payload, next_payload_data, (64 < sizeof(next_payload_data)) ? 64 : sizeof(next_payload_data)); };
    next->data = payload;

    // Link and free 'next' to create stale pointer
    parse_current->next = next;
    free(next);

    // Call entry; cil_gen_blockabstract will dereference parse_current->next->data (UAF)
    cil_gen_blockabstract(db, parse_current, ast_node);

    return 0;
}
