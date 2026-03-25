#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Prototype from harness
int cil_userprefixes_to_string(struct cil_db *db, char **out, size_t *size);

int main() {
    // Allocate db and list structures concretely
    struct cil_db *db = (struct cil_db *)calloc(1, sizeof(struct cil_db));
    struct cil_list *list = (struct cil_list *)calloc(1, sizeof(struct cil_list));
    struct cil_list_item *item = (struct cil_list_item *)calloc(1, sizeof(struct cil_list_item));

    // Wire the list into db
    db->userprefixes = list;
    list->head = item;

    // Create a tiny reclaimed buffer to simulate type confusion for curr->data
    // The vulnerable code treats this as struct cil_userprefix*, then reads fields
    unsigned char *reclaimed = (unsigned char *)malloc(1); // intentionally too small
    { static const unsigned char reclaimed_bytes_data[] = {0x00}; memcpy(reclaimed, reclaimed_bytes_data, (1 < sizeof(reclaimed_bytes_data)) ? 1 : sizeof(reclaimed_bytes_data)); };

    // Assign to list item data (void*)
    item->data = (void *)reclaimed;

    // out/size outputs
    char *out = NULL;
    size_t size = 0;

    // Call straight into the entry which directly calls the vulnerable function
    cil_userprefixes_to_string(db, &out, &size);

    return 0;
}
