// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <string.h>

/* Minimal compatible local types (must match harness preamble) */
typedef struct sepol_handle { int dummy; } sepol_handle_t;

typedef struct sepol_bool {
    char *name;
    int value;
} sepol_bool_t;

int sepol_bool_key_extract(sepol_handle_t *handle, const sepol_bool_t *boolean);

int main() {
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));
    sepol_bool_t *boolean = (sepol_bool_t *)calloc(1, sizeof(sepol_bool_t));

    const size_t N = 64;
    char *name_buf = (char *)malloc(N);
    { static const unsigned char name_buf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(name_buf, name_buf_data, (N < sizeof(name_buf_data)) ? N : sizeof(name_buf_data)); };
    name_buf[N-1] = '\0';

    // Create stale pointer: boolean->name points to freed memory
    boolean->name = name_buf;
    free(name_buf);

    sepol_bool_key_extract(handle, boolean);
    return 0;
}
