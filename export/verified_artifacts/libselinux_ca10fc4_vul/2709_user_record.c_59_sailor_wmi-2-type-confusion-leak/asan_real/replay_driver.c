#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Declarations from the harness
extern int sepol_user_key_unpack(sepol_user_key_t *key);
extern void sepol_user_key_free(sepol_user_key_t *key);

int main() {
    // Phase 1: allocate a real key object and its name buffer
    sepol_user_key_t *key = (sepol_user_key_t *)malloc(sizeof(sepol_user_key_t));
    if (!key) return 0;

    const size_t name_sz = 32; // concrete size as required
    char *name_buf = (char *)malloc(name_sz);
    if (!name_buf) return 0;
    { static const unsigned char name_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(name_buf, name_buf_data, (name_sz < sizeof(name_buf_data)) ? name_sz : sizeof(name_buf_data)); };
    // ensure it is a valid C-string (optional, not required for the bug)
    name_buf[name_sz - 1] = '\0';

    key->name = name_buf;

    // Phase 2: free the object to create a stale pointer for UAF/Type-Confusion path
    sepol_user_key_free(key);

    // Phase 3: use-after-free via stale pointer in vulnerable function
    sepol_user_key_unpack(key);

    return 0;
}
