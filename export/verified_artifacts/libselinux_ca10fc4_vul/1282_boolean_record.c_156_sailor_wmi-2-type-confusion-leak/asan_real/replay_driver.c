// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Minimal local typedefs matching harness
typedef struct sepol_handle { int dummy; } sepol_handle_t;
typedef struct sepol_bool { char *name; int value; } sepol_bool_t;

// Prototypes from harness
int sepol_bool_clone(sepol_handle_t *handle, const sepol_bool_t *boolean);
void sepol_bool_free(sepol_bool_t *boolean);

int main() {
    // Allocate handle
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));

    // Phase 1: allocate a sepol_bool and its name buffer
    sepol_bool_t *b = (sepol_bool_t *)calloc(1, sizeof(sepol_bool_t));
    char *namebuf = (char *)malloc(64);
    { static const unsigned char namebuf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(namebuf, namebuf_data, (64 < sizeof(namebuf_data)) ? 64 : sizeof(namebuf_data)); };
    // Ensure NUL-termination so strdup won't run off
    namebuf[63] = '\0';
    b->name = namebuf;
    int v; { static const unsigned char bool_value_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&v, bool_value_data, (sizeof(v) < sizeof(bool_value_data)) ? sizeof(v) : sizeof(bool_value_data)); }; b->value = v;

    // Keep a stale alias to the struct, then free the object (and its name)
    const sepol_bool_t *stale = b;
    sepol_bool_free(b);  // frees b->name and b

    // Optional allocator noise to encourage reuse (not required)
    void *noise = malloc(32);
    (void)noise;

    // Phase 2: use-after-free via clone path
    sepol_bool_clone(handle, stale);
    return 0;
}
