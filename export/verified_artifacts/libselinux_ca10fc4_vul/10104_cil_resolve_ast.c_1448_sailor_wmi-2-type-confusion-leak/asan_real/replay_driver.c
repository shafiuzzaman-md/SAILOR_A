#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Entry declaration from harness
int cil_resolve_classorder(struct cil_tree_node *current, struct cil_db *db, struct cil_list *classorder_list, struct cil_list *unordered_classorder_list);

int main(void) {
    // Allocate core objects with concrete sizes
    struct cil_tree_node *current = (struct cil_tree_node *)calloc(1, sizeof(struct cil_tree_node));
    struct cil_ordered *ordered = (struct cil_ordered *)calloc(1, sizeof(struct cil_ordered));
    struct cil_db *db = (struct cil_db *)calloc(1, sizeof(struct cil_db));
    struct cil_list *classorder_list = (struct cil_list *)calloc(1, sizeof(struct cil_list));
    struct cil_list *unordered_classorder_list = (struct cil_list *)calloc(1, sizeof(struct cil_list));

    // Build a cil_list for ordered->strs with a head item
    struct cil_list *strs = (struct cil_list *)malloc(sizeof(struct cil_list));
    memset(strs, 0, sizeof(*strs));

    struct cil_list_item *head = (struct cil_list_item *)malloc(sizeof(struct cil_list_item));
    memset(head, 0, sizeof(*head));
    // Make head content symbolic (not strictly needed for UAF, but ok for exploration)
    { static const unsigned char strs_head_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(head, strs_head_data, (sizeof(*head) < sizeof(strs_head_data)) ? sizeof(*head) : sizeof(strs_head_data)); };

    strs->head = head;
    strs->tail = NULL;
    strs->flavor = 0;

    // Wire up current -> ordered -> strs
    ordered->strs = strs;
    ordered->datums = NULL;
    current->data = ordered;

    // Create UAF: free the list structure so cil_list_for_each dereferences a freed pointer
    free(strs);

    // Call entry (pass-through to cil_resolve_classorder)
    cil_resolve_classorder(current, db, classorder_list, unordered_classorder_list);

    return 0;
}
