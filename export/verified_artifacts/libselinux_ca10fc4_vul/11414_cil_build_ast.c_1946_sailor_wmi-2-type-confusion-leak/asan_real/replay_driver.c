#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int cil_gen_roleallow(struct cil_db *db, struct cil_tree_node *parse_current, struct cil_tree_node *ast_node);

int main() {
    // 1) Concrete allocations
    struct cil_db *db = (struct cil_db *)calloc(1, sizeof(struct cil_db));
    struct cil_tree_node *parse_current = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *n1 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *n2 = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *ast_node = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));

    // 2) Link the chain: parse_current -> n1 -> n2
    parse_current->next = n1;
    n1->next = n2;
    n2->next = NULL;

    // 3) Data buffers for n1 and n2 (symbolic contents, concrete sizes)
    enum { BUF_SZ = 64 };
    char *d1 = (char *)malloc(BUF_SZ);
    char *d2 = (char *)malloc(BUF_SZ);
    { static const unsigned char d1_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(d1, d1_bytes_data, (BUF_SZ < sizeof(d1_bytes_data)) ? BUF_SZ : sizeof(d1_bytes_data)); };
    { static const unsigned char d2_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(d2, d2_bytes_data, (BUF_SZ < sizeof(d2_bytes_data)) ? BUF_SZ : sizeof(d2_bytes_data)); };
    n1->data = d1;
    n2->data = d2;

    // 4) Make scalar fields symbolic via temporaries, then assign
    int fl0, fl1, fl2;
    memset(&fl0, 0, sizeof(fl0)); /* replay: no ktest data for "flavor0" */;
    memset(&fl1, 0, sizeof(fl1)); /* replay: no ktest data for "flavor1" */;
    memset(&fl2, 0, sizeof(fl2)); /* replay: no ktest data for "flavor2" */;
    parse_current->flavor = fl0;
    n1->flavor = fl1;
    n2->flavor = fl2;

    // 5) Call entry (direct pass-through to vulnerable function)
    (void)cil_gen_roleallow(db, parse_current, ast_node);
    return 0;
}
