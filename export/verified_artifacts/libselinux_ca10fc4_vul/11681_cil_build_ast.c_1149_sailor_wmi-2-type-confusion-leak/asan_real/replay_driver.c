#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int cil_gen_classcommon(struct cil_db *db, struct cil_tree_node *parse_current, struct cil_tree_node *ast_node);

int main() {
    struct cil_db *db = (struct cil_db *)calloc(1, sizeof(struct cil_db));
    struct cil_tree_node *cur = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *n1 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *n2 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *ast = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));

    // Link parse_current -> next -> next
    cur->next = n1;
    n1->next = n2;

    // Prepare data fields (strings)
    char *class_buf = (char *)malloc(32);
    char *common_buf = (char *)malloc(32);
    { static const unsigned char class_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(class_buf, class_buf_data, (32 < sizeof(class_buf_data)) ? 32 : sizeof(class_buf_data)); };
    { static const unsigned char common_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(common_buf, common_buf_data, (32 < sizeof(common_buf_data)) ? 32 : sizeof(common_buf_data)); };
    class_buf[31] = '\0';
    common_buf[31] = '\0';

    n1->data = class_buf;
    n2->data = common_buf;

    // Call entry directly
    cil_gen_classcommon(db, cur, ast);
    return 0;
}
