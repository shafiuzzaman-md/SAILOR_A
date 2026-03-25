#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int cil_gen_blockabstract(struct cil_db *db, struct cil_tree_node *parse_current, struct cil_tree_node *ast_node);

int main() {
    // Allocate concrete objects
    void *db_buf = calloc(1, 256);  // opaque allocation for incomplete struct
    struct cil_db *db = (struct cil_db *)db_buf;
    struct cil_tree_node *parse_current = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *next = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_tree_node *ast_node = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));

    // Allocate a data buffer to simulate parse_current->next->data
    enum { DATA_BUF_SZ = 64 };
    void *data_buf = malloc(DATA_BUF_SZ);
    { static const unsigned char next_data_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(data_buf, next_data_buf_data, (DATA_BUF_SZ < sizeof(next_data_buf_data)) ? DATA_BUF_SZ : sizeof(next_data_buf_data)); };

    // Wire up the parse tree linkage expected by the vulnerable line
    parse_current->next = next;
    next->data = data_buf;  // will be read and assigned to abstract->block_str

    // Set concrete values for fields; symbolic not required for this path
    parse_current->line = 1;
    next->line = 1;

    // Call entry (pass-through to cil_gen_blockabstract)
    cil_gen_blockabstract(db, parse_current, ast_node);

    return 0;
}
