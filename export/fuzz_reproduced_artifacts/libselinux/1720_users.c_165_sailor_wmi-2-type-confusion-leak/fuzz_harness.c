#include <stdint.h>
#include <stddef.h>
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

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 28) return 0;
    // Allocate concrete objects for params
    sepol_handle_t *handle = (sepol_handle_t *)malloc(sizeof(sepol_handle_t));
    sepol_policydb_t *p = (sepol_policydb_t *)malloc(sizeof(sepol_policydb_t));
    sepol_user_key_t *key = (sepol_user_key_t *)malloc(sizeof(sepol_user_key_t));
    sepol_user_t *user = (sepol_user_t *)malloc(sizeof(sepol_user_t));

    // Make contents symbolic so KLEE can explore values
    { memcpy(handle, fuzz_data + 0, 16); };
    { memcpy(p, fuzz_data + 16, 4); };
    { memcpy(key, fuzz_data + 20, 4); };
    { memcpy(user, fuzz_data + 24, 4); };

    // Avoid indirect call via msg_callback so we only model the read
    handle->msg_callback = 0; // NULL

    // Free the handle to create a stale pointer (UAF/type-confusion read)
    free(handle);

    // Call entry which directly calls sepol_user_modify (no guards in harness)
    sepol_user_modify(handle, p, key, user);

    return 0;
}
