#include <string.h>
#include "harness_types.h"
#include <stdlib.h>
#include <stdio.h>
// klee removed for replay

// Prototypes from harness
int sepol_policy_file_get_len(sepol_policy_file_t *spf, size_t *out_len);
void sepol_policy_file_free(sepol_policy_file_t * pf);

int main() {
    // Phase 1: allocate and initialize a valid sepol_policy_file_t
    sepol_policy_file_t *spf = (sepol_policy_file_t *)calloc(1, sizeof(*spf));
    if (!spf) return 0;

    // Initialize to pass the PF_LEN guard (if reached after free reuse)
    spf->pf.type = PF_LEN;
    spf->pf.len = 64;
    spf->pf.size = 64;
    spf->pf.data = malloc(64);

    // Optional: make buffer symbolic (not strictly needed for UAF)
    if (spf->pf.data) {
        { static const unsigned char policy_data_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(spf->pf.data, policy_data_data, (64 < sizeof(policy_data_data)) ? 64 : sizeof(policy_data_data)); };
    }

    size_t out_len = 0;

    // Phase 2: free the object to create a stale pointer
    sepol_policy_file_free(spf);

    // Phase 3: use-after-free via stale pointer
    // This will cause a load from freed memory inside sepol_policy_file_get_len
    // (pf->type and/or pf->len), which KLEE flags as .free.err
    sepol_policy_file_get_len(spf, &out_len);

    return 0;
}
