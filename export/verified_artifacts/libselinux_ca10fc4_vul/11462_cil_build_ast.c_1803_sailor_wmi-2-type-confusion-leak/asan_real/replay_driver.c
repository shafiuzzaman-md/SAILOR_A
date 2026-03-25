#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int cil_gen_roletype(struct cil_db *db, struct cil_tree_node *parse_current, struct cil_tree_node *ast_node);

int main() {
    // Allocate concrete objects
    struct cil_db *db = (struct cil_db *)calloc(1, sizeof(struct cil_db));
    struct cil_tree_node *parse_current = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *node1 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *node2 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *ast_node = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));

    // Build the next chain: parse_current -> node1 -> node2
    parse_current->next = node1;
    node1->next = node2;

    // Allocate buffers for role_str and type_str and make them symbolic
    const size_t BUF_SZ = 64;  // concrete size as required
    char *role_buf = (char *)malloc(BUF_SZ);
    char *type_buf = (char *)malloc(BUF_SZ);
    { static const unsigned char role_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(role_buf, role_buf_data, (BUF_SZ < sizeof(role_buf_data)) ? BUF_SZ : sizeof(role_buf_data)); };
    { static const unsigned char type_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(type_buf, type_buf_data, (BUF_SZ < sizeof(type_buf_data)) ? BUF_SZ : sizeof(type_buf_data)); };

    // Ensure NUL-termination to avoid library surprises if used
    role_buf[BUF_SZ - 1] = '\0';
    type_buf[BUF_SZ - 1] = '\0';

    // Set the data fields to point to the buffers
    node1->data = (void *)role_buf;
    node2->data = (void *)type_buf;

    // UAF setup for WMI-2: free the second node so that
    //   roletype->type_str = parse_current->next->next->data;
    // dereferences a freed object (node2)
    free(node2);

    // IMPORTANT: Do NOT allocate anything else here; we want a .free.err on deref

    // Call the entry function (pass-through to vulnerable function)
    cil_gen_roletype(db, parse_current, ast_node);

    return 0;
}
