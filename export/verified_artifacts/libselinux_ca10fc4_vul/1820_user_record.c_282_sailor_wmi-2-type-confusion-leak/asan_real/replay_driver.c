#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Prototype from harness
int sepol_user_get_roles(sepol_handle_t *handle, const sepol_user_t *user,
               const char ***roles_arr, unsigned int *num_roles);

int main() {
    // Allocate concrete handle and user without using calloc (we stub calloc to NULL)
    sepol_handle_t *handle = (sepol_handle_t *)malloc(sizeof(sepol_handle_t));
    sepol_user_t *user = (sepol_user_t *)malloc(sizeof(sepol_user_t));
    if (!handle || !user) return 0;
    memset(handle, 0, sizeof(*handle));
    memset(user, 0, sizeof(*user));

    // Prepare handle fields potentially read by ERR/logging path
    void *cb = malloc(16);
    void *cb_arg = malloc(16);
    handle->msg_callback = cb;
    handle->msg_callback_arg = cb_arg;

    // Prepare user->name used in the ERR format string
    char *uname = (char *)malloc(32);
    { static const unsigned char user_name_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(uname, user_name_data, (32 < sizeof(user_name_data)) ? 32 : sizeof(user_name_data)); };
    uname[31] = '\0';
    user->name = uname;

    // num_roles can be anything; calloc is stubbed to return NULL, so omem path is taken
    user->num_roles = 3;
    user->roles = NULL;  // not used because we jump to omem before the loop

    const char **roles_out = NULL;
    unsigned int num_out = 0;

    // Direct call via entry to the vulnerable function
    sepol_user_get_roles(handle, user, &roles_out, &num_out);
    return 0;
}
