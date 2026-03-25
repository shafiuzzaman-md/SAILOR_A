#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

/* entry function from harness */
extern int sepol_user_compare2(const sepol_user_t *user, const sepol_user_t *user2);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 128) return 0;
    // Allocate two users concretely
    sepol_user_t *u1 = (sepol_user_t *)calloc(1, sizeof(sepol_user_t));
    sepol_user_t *u2 = (sepol_user_t *)calloc(1, sizeof(sepol_user_t));

    // Allocate name buffers with concrete sizes
    char *n1 = (char *)malloc(64);
    char *n2 = (char *)malloc(64);

    // Make contents symbolic and ensure NUL-termination for strcmp
    { memcpy(n1, fuzz_data + 0, 64); };
    { memcpy(n2, fuzz_data + 64, 64); };
    n1[63] = '\0';
    n2[63] = '\0';

    // Assign into structs
    u1->name = n1;
    u2->name = n2;

    // Create UAF on the first user to exercise read of freed memory in strcmp
    free(u1);

    // Call entry directly (no guards)
    sepol_user_compare2(u1, u2);
    return 0;
}
