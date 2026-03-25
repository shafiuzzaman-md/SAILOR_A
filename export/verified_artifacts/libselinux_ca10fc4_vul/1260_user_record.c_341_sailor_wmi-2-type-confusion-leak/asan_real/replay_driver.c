// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Minimal matching types (must match harness definitions)
typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_user {
    char *name;
    char *mls_level;
    char *mls_range;
    char **roles;
    unsigned int num_roles;
} sepol_user_t;

// entry_func prototype from harness
int sepol_user_clone(sepol_handle_t *handle, const sepol_user_t *user, sepol_user_t **user_ptr);

int main() {
    // Concrete allocations (no symbolic sizes)
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));

    // Allocate a sepol_user_t object
    sepol_user_t *user = (sepol_user_t *)malloc(sizeof(sepol_user_t));
    // Populate fields
    user->name = (char *)malloc(16);
    { static const unsigned char user_name_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(user->name, user_name_data, (16 < sizeof(user_name_data)) ? 16 : sizeof(user_name_data)); };
    user->mls_level = NULL;
    user->mls_range = NULL;
    user->num_roles = 1;
    user->roles = (char **)malloc(sizeof(char*) * user->num_roles);
    user->roles[0] = (char *)malloc(8);
    { static const unsigned char role0_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(user->roles[0], role0_data, (8 < sizeof(role0_data)) ? 8 : sizeof(role0_data)); };

    // Free the container to create a stale pointer (WMI-2 pattern)
    free(user);

    // Reclaim memory with a different object size to encourage reuse
    void *reclaim = malloc(64);
    { static const unsigned char reclaim_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(reclaim, reclaim_buf_data, (64 < sizeof(reclaim_buf_data)) ? 64 : sizeof(reclaim_buf_data)); };
    (void)reclaim;

    sepol_user_t *out_user = NULL;
    // Call entry -> vulnerable path dereferences freed 'user'
    sepol_user_clone(handle, user, &out_user);
    return 0;
}
