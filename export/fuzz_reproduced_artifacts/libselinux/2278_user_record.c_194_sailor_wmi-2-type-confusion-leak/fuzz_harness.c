#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

/* Prototypes from harness/user_record.c */
int sepol_user_add_role(sepol_handle_t *handle, sepol_user_t *user, const char *role);
void sepol_user_free(sepol_user_t *user);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 16) return 0;
    // Allocate concrete objects
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));
    sepol_user_t *user = (sepol_user_t *)calloc(1, sizeof(sepol_user_t));

    // Initialize fields to a consistent state prior to free
    // Set roles=NULL and num_roles=0 so reallocarray() succeeds predictably
    user->roles = NULL;
    user->num_roles = 0;

    // Prepare a role string (symbolic contents, concrete size) so strdup() succeeds
    char role_buf[16];
    { memcpy(role_buf, fuzz_data + 0, 16); };
    role_buf[15] = '\0';

    // Phase 1: Free the user object — creates a dangling pointer (UAF setup)
    sepol_user_free(user);

    // Phase 2: Use-after-free — dereference freed 'user' inside add_role
    // This reaches the vulnerable statement and should trigger KLEE's error
    (void)sepol_user_add_role(handle, user, role_buf);

    return 0;
}
