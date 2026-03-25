#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Prototypes from harness/user_record.c
extern int sepol_user_get_roles(sepol_handle_t *handle,
                      const sepol_user_t *user,
                      const char ***roles_arr,
                      unsigned int *num_roles);
extern void sepol_user_free(sepol_user_t *user);

int main() {
    // Allocate handle and user concretely
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(*handle));

    sepol_user_t *user = (sepol_user_t *)calloc(1, sizeof(*user));

    // Initialize fields so free() can safely walk them, but values are mostly symbolic
    user->num_roles = 1;  // small, concrete to bound allocations/loop

    user->roles = (char **)calloc(1, sizeof(char *));
    char *role0 = (char *)malloc(8);
    { static const unsigned char role0_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(role0, role0_bytes_data, (8 < sizeof(role0_bytes_data)) ? 8 : sizeof(role0_bytes_data)); };
    role0[7] = 0;  // make it a string just in case
    user->roles[0] = role0;

    char *name = (char *)malloc(8);
    { static const unsigned char name_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(name, name_bytes_data, (8 < sizeof(name_bytes_data)) ? 8 : sizeof(name_bytes_data)); };
    name[7] = 0;
    user->name = name;

    user->mls_level = (char *)malloc(4);
    user->mls_range = (char *)malloc(4);

    // Keep a stale reference, then free the real object
    const sepol_user_t *stale = user;
    sepol_user_free(user);

    // Call into the entry function with the stale pointer to trigger UAF on read
    const char **out_roles = NULL;
    unsigned int out_num = 0;

    sepol_user_get_roles(handle, stale, &out_roles, &out_num);
    return 0;
}
