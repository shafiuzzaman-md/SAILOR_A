#include <stdint.h>
#include <stddef.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Minimal compatible type defs (must match harness layouts)
typedef void (*sepol_msg_callback_t)(void *arg, const char *msg);

typedef struct sepol_handle {
    sepol_msg_callback_t msg_callback;
    void *msg_callback_arg;
} sepol_handle_t;

typedef struct sepol_bool {
    char *name;
    int value;
} sepol_bool_t;

// entry from harness
int sepol_bool_set_name(sepol_handle_t *handle, sepol_bool_t *boolean, const char *name);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 48) return 0;
    // Allocate handle and make its bytes symbolic (potential leak/UAF target)
    sepol_handle_t *handle = (sepol_handle_t *)malloc(sizeof(sepol_handle_t));
    if (!handle) return 0;
    { memcpy(handle, fuzz_data + 0, 16); };

    // Allocate boolean and its current name
    sepol_bool_t *boolean = (sepol_bool_t *)calloc(1, sizeof(sepol_bool_t));
    if (!boolean) return 0;
    char *oldname = (char *)malloc(16);
    if (!oldname) return 0;
    { memcpy(oldname, fuzz_data + 16, 16); };
    oldname[15] = '\0';
    boolean->name = oldname;

    // Prepare name input buffer (for strdup), concrete size, symbolic content
    char namebuf[16];
    { memcpy(namebuf, fuzz_data + 32, 16); };
    namebuf[15] = '\0';

    // Model stale reference/UAF: destroy the handle before use to create stale read in ERR
    free(handle);

    // Call entry directly (no guards)
    sepol_bool_set_name(handle, boolean, namebuf);

    return 0;
}
