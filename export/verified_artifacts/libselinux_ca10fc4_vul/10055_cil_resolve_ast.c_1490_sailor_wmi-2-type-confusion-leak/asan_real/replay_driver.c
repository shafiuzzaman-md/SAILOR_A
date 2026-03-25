#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int cil_resolve_sidorder(struct cil_tree_node *current, struct cil_db *db, struct cil_list *sidorder_list);

int main() {
    // Allocate objects with concrete sizes
    struct cil_tree_node *current = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_db *db = (struct cil_db *)calloc(1, sizeof(struct cil_db));
    struct cil_list *sidorder_list = (struct cil_list *)calloc(1, sizeof(struct cil_list));

    // Build an ordered container
    struct cil_ordered *ordered = (struct cil_ordered *)calloc(1, sizeof(struct cil_ordered));
    current->data = ordered;

    // Create a cil_list with at least one item, then FREE it to simulate a stale pointer (UAF / type confusion reclaim site)
    struct cil_list *strs = (struct cil_list *)malloc(sizeof(struct cil_list));
    struct cil_list_item *item = (struct cil_list_item *)malloc(sizeof(struct cil_list_item));

    // Make the list item contents symbolic so KLEE can explore values
    { static const unsigned char sidorder_item_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(item, sidorder_item_data, (sizeof(*item) < sizeof(sidorder_item_data)) ? sizeof(*item) : sizeof(sidorder_item_data)); };
    item->next = NULL;  // keep a simple single-item list

    strs->head = item;

    // Free the list container to create a dangling pointer in ordered->strs
    free(strs);

    // Assign the stale pointer; dereference happens in cil_list_for_each in cil_resolve_sidorder
    ordered->strs = strs;     // STALE pointer (freed above)
    ordered->datums = NULL;   // unused in sliced harness

    // Call the entry which directly invokes the vulnerable function
    cil_resolve_sidorder(current, db, sidorder_list);

    return 0;
}
