// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Minimal local replicas matching harness/users.c
typedef struct sepol_handle {
    void (*msg_callback)(void *, const char *);
    void *msg_callback_arg;
} sepol_handle_t;

typedef struct { int dummy; } sepol_policydb_t;
typedef struct { int dummy; } sepol_user_key_t;
typedef struct { int dummy; } sepol_user_t;

int sepol_user_modify(sepol_handle_t *handle,
                sepol_policydb_t *p,
                const sepol_user_key_t *key,
                const sepol_user_t *user);

int main() {
    // Allocate concrete objects for params
    sepol_handle_t *handle = (sepol_handle_t *)malloc(sizeof(sepol_handle_t));
    sepol_policydb_t *p = (sepol_policydb_t *)malloc(sizeof(sepol_policydb_t));
    sepol_user_key_t *key = (sepol_user_key_t *)malloc(sizeof(sepol_user_key_t));
    sepol_user_t *user = (sepol_user_t *)malloc(sizeof(sepol_user_t));

    // Make contents symbolic so KLEE can explore values
    { static const unsigned char handle_sym_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(handle, handle_sym_data, (sizeof(*handle) < sizeof(handle_sym_data)) ? sizeof(*handle) : sizeof(handle_sym_data)); };
    { static const unsigned char policydb_sym_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(p, policydb_sym_data, (sizeof(*p) < sizeof(policydb_sym_data)) ? sizeof(*p) : sizeof(policydb_sym_data)); };
    { static const unsigned char user_key_sym_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(key, user_key_sym_data, (sizeof(*key) < sizeof(user_key_sym_data)) ? sizeof(*key) : sizeof(user_key_sym_data)); };
    { static const unsigned char user_sym_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(user, user_sym_data, (sizeof(*user) < sizeof(user_sym_data)) ? sizeof(*user) : sizeof(user_sym_data)); };

    // Avoid indirect call via msg_callback so we only model the read
    handle->msg_callback = 0; // NULL

    // Free the handle to create a stale pointer (UAF/type-confusion read)
    free(handle);

    // Call entry which directly calls sepol_user_modify (no guards in harness)
    sepol_user_modify(handle, p, key, user);

    return 0;
}
