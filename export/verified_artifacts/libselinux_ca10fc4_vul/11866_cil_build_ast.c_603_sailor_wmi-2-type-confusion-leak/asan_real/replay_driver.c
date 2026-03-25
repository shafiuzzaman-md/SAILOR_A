#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
// klee removed for replay

// Match the minimal types defined in harness/cil_build_ast.c
struct cil_db { int dummy; };
struct cil_tree_node { struct cil_tree_node *next; struct cil_tree_node *cl_head; };

extern int cil_gen_class(struct cil_db *db, struct cil_tree_node *parse_current, struct cil_tree_node *ast_node);

int main() {
    // Allocate minimal objects
    struct cil_db *db = (struct cil_db *)calloc(1, sizeof(struct cil_db));
    struct cil_tree_node *n0 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *ast = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));

    // Make n0 symbolic
    { static const unsigned char n0_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(n0, n0_data, (sizeof(*n0) < sizeof(n0_data)) ? sizeof(*n0) : sizeof(n0_data)); };

    // Set first-level next concretely to proceed, second-level next remains symbolic to induce invalid deref
    struct cil_tree_node *p1;
    { static const unsigned char p1_sym_data[] = {0xd0, 0x86, 0xcb, 0xea, 0xcd, 0x7f, 0x00, 0x00}; memcpy(&p1, p1_sym_data, (sizeof(p1) < sizeof(p1_sym_data)) ? sizeof(p1) : sizeof(p1_sym_data)); };
    n0->next = p1;  // symbolic pointer; dereferencing p1->next will trigger a pointer error at the vulnerable statement

    // Call entry
    cil_gen_class(db, n0, ast);
    return 0;
}
