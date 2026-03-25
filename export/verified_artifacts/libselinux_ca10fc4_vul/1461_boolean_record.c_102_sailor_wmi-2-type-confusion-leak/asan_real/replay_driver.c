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

int main() {
    // Allocate handle and make its bytes symbolic (potential leak/UAF target)
    sepol_handle_t *handle = (sepol_handle_t *)malloc(sizeof(sepol_handle_t));
    if (!handle) return 0;
    { static const unsigned char handle_sym_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(handle, handle_sym_data, (sizeof(*handle) < sizeof(handle_sym_data)) ? sizeof(*handle) : sizeof(handle_sym_data)); };

    // Allocate boolean and its current name
    sepol_bool_t *boolean = (sepol_bool_t *)calloc(1, sizeof(sepol_bool_t));
    if (!boolean) return 0;
    char *oldname = (char *)malloc(16);
    if (!oldname) return 0;
    { static const unsigned char oldname_sym_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(oldname, oldname_sym_data, (16 < sizeof(oldname_sym_data)) ? 16 : sizeof(oldname_sym_data)); };
    oldname[15] = '\0';
    boolean->name = oldname;

    // Prepare name input buffer (for strdup), concrete size, symbolic content
    char namebuf[16];
    { static const unsigned char namebuf_sym_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(namebuf, namebuf_sym_data, (sizeof(namebuf) < sizeof(namebuf_sym_data)) ? sizeof(namebuf) : sizeof(namebuf_sym_data)); };
    namebuf[15] = '\0';

    // Model stale reference/UAF: destroy the handle before use to create stale read in ERR
    free(handle);

    // Call entry directly (no guards)
    sepol_bool_set_name(handle, boolean, namebuf);

    return 0;
}
