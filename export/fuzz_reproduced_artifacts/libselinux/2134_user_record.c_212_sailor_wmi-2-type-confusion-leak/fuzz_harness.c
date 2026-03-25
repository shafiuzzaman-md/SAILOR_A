#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// externs from harness/user_record.c
extern int sepol_user_has_role(struct sepol_user *user, const char *role);
extern void sepol_user_free(struct sepol_user *user);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 8) return 0;
    // Allocate a real sepol_user object and populate fields
    struct sepol_user *user = (struct sepol_user *)malloc(sizeof(struct sepol_user));
    if (!user) return 0;

    // Allocate a roles array with at least 1 entry so loop condition would be true if not freed
    unsigned int n = 1;
    user->num_roles = n;

    user->name = (char *)malloc(16);
    user->mls_level = (char *)malloc(16);
    user->mls_range = (char *)malloc(16);

    user->roles = (char **)calloc(n, sizeof(char *));
    for (unsigned int i = 0; i < n; ++i) {
        user->roles[i] = (char *)malloc(8);
        // make role content symbolic so strcmp path can be explored if not crashing earlier
        { static const unsigned char role_entry_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(user->roles[i], role_entry_data, (8 < sizeof(role_entry_data)) ? 8 : sizeof(role_entry_data)); };
        // ensure zero-termination to avoid strcmp running off
        user->roles[i][7] = '\0';
    }

    // Create a role input string (separate from user->roles)
    char role_buf[8];
    { memcpy(role_buf, fuzz_data + 0, 8); };
    role_buf[7] = '\0';

    // Phase 1: Free the user and its internals
    sepol_user_free(user);

    // Phase 2: Use-after-free — pass the stale pointer to the entry
    // KLEE should detect .free.err on reading user->num_roles or user->roles[i]
    sepol_user_has_role(user, role_buf);

    return 0;
}
