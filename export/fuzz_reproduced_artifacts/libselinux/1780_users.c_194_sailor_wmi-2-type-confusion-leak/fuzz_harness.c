#include <stdint.h>
#include <stddef.h>
#include <string.h>
// NO_HARNESS_TYPES
#include <stdlib.h>
// klee removed for replay

// Opaque forward declarations matching harness parameter tags
struct sepol_handle_t;
struct sepol_policydb_t;
struct sepol_user_key_t;
struct sepol_user_t;

// Declare entry with opaque pointer params (types need not match exactly across TUs)
int sepol_user_modify(struct sepol_handle_t *handle,
          struct sepol_policydb_t *p,
          const struct sepol_user_key_t *key,
          const struct sepol_user_t *user);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 256) return 0;
    // Concrete buffer sizes for opaque objects
    enum { HSIZE = 64, PSIZE = 64, KSIZE = 64, USIZE = 64 };

    // Allocate concrete buffers and make contents symbolic
    char *hbuf = (char*)malloc(HSIZE);
    char *pbuf = (char*)malloc(PSIZE);
    char *kbuf = (char*)malloc(KSIZE);
    char *ubuf = (char*)malloc(USIZE);

    { memcpy(hbuf, fuzz_data + 0, 64); };
    { memcpy(pbuf, fuzz_data + 64, 64); };
    { memcpy(kbuf, fuzz_data + 128, 64); };
    { memcpy(ubuf, fuzz_data + 192, 64); };

    struct sepol_handle_t *handle = (struct sepol_handle_t*)hbuf;
    struct sepol_policydb_t *p = (struct sepol_policydb_t*)pbuf;
    struct sepol_user_key_t *key = (struct sepol_user_key_t*)kbuf;
    struct sepol_user_t *user = (struct sepol_user_t*)ubuf;

    // Phase 1: Free the handle to set up UAF (ERR(handle, ...) will deref it)
    free(handle);

    // Phase 2: Call entry directly — no guards
    sepol_user_modify(handle, p, key, user);
    return 0;
}
