// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

struct sepol_handle { int dummy; };
struct sepol_user {
    char *name;
    char *mls_level;
};

int sepol_user_clone(struct sepol_handle *handle, const struct sepol_user *user); // from harness

int main() {
    struct sepol_handle *handle = (struct sepol_handle *)calloc(1, sizeof(struct sepol_handle));

    struct sepol_user *user = (struct sepol_user *)calloc(1, sizeof(struct sepol_user));

    char *lvl = (char *)malloc(16);
    { static const unsigned char mls_level_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(lvl, mls_level_buf_data, (16 < sizeof(mls_level_buf_data)) ? 16 : sizeof(mls_level_buf_data)); };
    user->mls_level = lvl;

    char *name = (char *)malloc(8);
    { static const unsigned char user_name_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(name, user_name_data, (8 < sizeof(user_name_data)) ? 8 : sizeof(user_name_data)); };
    user->name = name;

    // Model UAF/Type-confusion: free the user object, keep stale pointer
    free(user);

    // Optionally reclaim memory to increase aliasing pressure
    void *reclaim = malloc(sizeof(struct sepol_user));
    (void)reclaim;

    sepol_user_clone(handle, (const struct sepol_user *)user);
    return 0;
}
