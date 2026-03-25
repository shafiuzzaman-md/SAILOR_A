#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// klee removed for replay

int cil_userprefixes_to_string(struct cil_db *db, char **out, size_t *size);
void cil_destroy_userprefix(struct cil_userprefix *p);

int main() {
    // Allocate db and its userprefixes list with one item
    struct cil_db *db = (struct cil_db *)calloc(1, sizeof(struct cil_db));
    struct cil_list *lst = (struct cil_list *)calloc(1, sizeof(struct cil_list));
    struct cil_list_item *it = (struct cil_list_item *)calloc(1, sizeof(struct cil_list_item));
    db->userprefixes = lst;
    lst->head = it;

    // Allocate cil_userprefix object that will be freed to create UAF
    struct cil_userprefix *up = (struct cil_userprefix *)calloc(1, sizeof(struct cil_userprefix));
    struct cil_user *user = (struct cil_user *)calloc(1, sizeof(struct cil_user));

    // Allocate and fill strings (symbolic content, NUL-terminated)
    const size_t STR_SZ = 32;
    char *fqn = (char *)malloc(STR_SZ);
    char *prefix = (char *)malloc(STR_SZ);
    { static const unsigned char user_fqn_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(fqn, user_fqn_data, (STR_SZ < sizeof(user_fqn_data)) ? STR_SZ : sizeof(user_fqn_data)); };
    { static const unsigned char prefix_str_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(prefix, prefix_str_data, (STR_SZ < sizeof(prefix_str_data)) ? STR_SZ : sizeof(prefix_str_data)); };
    fqn[STR_SZ - 1] = '\0';
    prefix[STR_SZ - 1] = '\0';

    user->datum.fqn = fqn;
    up->user = user;
    up->prefix_str = prefix;

    it->data = up;
    it->next = NULL;

    // Phase 1: free the cil_userprefix (and its prefix) but leave stale list reference
    cil_destroy_userprefix(up);

    // Phase 2: call into vulnerable path which dereferences the freed pointer via the list
    char *out = NULL;
    size_t size = 0;
    cil_userprefixes_to_string(db, &out, &size);

    return 0;
}
