#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// entry function from harness
int sepol_user_free(sepol_user_t *user);

int main() {
    // Allocate user struct concretely
    sepol_user_t *user = (sepol_user_t *)calloc(1, sizeof(sepol_user_t));

    // Initialize fields with concrete allocations
    user->name = (char *)malloc(16);
    user->mls_level = (char *)malloc(16);
    user->mls_range = (char *)malloc(16);

    // Make contents symbolic (not sizes)
    { static const unsigned char user_name_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(user->name, user_name_data, (16 < sizeof(user_name_data)) ? 16 : sizeof(user_name_data)); };
    { static const unsigned char user_mls_level_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(user->mls_level, user_mls_level_data, (16 < sizeof(user_mls_level_data)) ? 16 : sizeof(user_mls_level_data)); };
    { static const unsigned char user_mls_range_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(user->mls_range, user_mls_range_data, (16 < sizeof(user_mls_range_data)) ? 16 : sizeof(user_mls_range_data)); };

    // Setup roles array with two roles
    user->num_roles = 2;
    user->roles = (char **)calloc(user->num_roles, sizeof(char *));
    user->roles[0] = (char *)malloc(16);
    user->roles[1] = (char *)malloc(16);
    { static const unsigned char role0_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(user->roles[0], role0_data, (16 < sizeof(role0_data)) ? 16 : sizeof(role0_data)); };
    { static const unsigned char role1_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(user->roles[1], role1_data, (16 < sizeof(role1_data)) ? 16 : sizeof(role1_data)); };

    // First call frees the struct and its fields
    sepol_user_free(user);
    // Second call dereferences freed struct -> UAF
    sepol_user_free(user);

    return 0;
}
