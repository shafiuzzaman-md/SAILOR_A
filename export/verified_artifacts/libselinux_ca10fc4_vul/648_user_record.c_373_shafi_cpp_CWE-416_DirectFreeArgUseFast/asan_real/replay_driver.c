#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

// Entry provided by harness
int sepol_user_free(sepol_user_t *user);

int main() {
    // Allocate user object
    sepol_user_t *user = (sepol_user_t *)calloc(1, sizeof(sepol_user_t));

    // Set up fields
    user->num_roles = 2;  // ensure loop executes twice

    // Allocate roles array with two entries
    user->roles = (char **)calloc(2, sizeof(char *));

    // Allocate a single role string and alias it in both slots to trigger double-free on second iteration
    char *rolebuf = (char *)malloc(16);
    if (!rolebuf) return 0;
    { static const unsigned char rolebuf_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(rolebuf, rolebuf_bytes_data, (16 < sizeof(rolebuf_bytes_data)) ? 16 : sizeof(rolebuf_bytes_data)); };  // content symbolic (size is concrete)
    rolebuf[15] = '\0';

    user->roles[0] = rolebuf;
    user->roles[1] = rolebuf;  // alias to cause second free of same pointer

    // Other fields that sepol_user_free() will free
    user->name = (char *)malloc(8);
    if (user->name) memset(user->name, 0x41, 8);
    user->mls_level = malloc(8);
    user->mls_range = malloc(8);

    // Direct pass-through call to vulnerable function via harness entry
    sepol_user_free(user);
    return 0;
}
