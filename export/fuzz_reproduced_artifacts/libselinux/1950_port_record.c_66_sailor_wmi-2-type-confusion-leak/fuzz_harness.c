#include <stddef.h>
#include <string.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// Minimal compatible type defs (match harness/port_record.c)
typedef struct sepol_handle { int dummy; } sepol_handle_t;
typedef struct sepol_context { int dummy; } sepol_context_t;

typedef struct sepol_port {
    int low, high;
    int proto;
    sepol_context_t *con;
} sepol_port_t;

typedef struct sepol_port_key {
    int low, high;
    int proto;
} sepol_port_key_t;

// entry prototype from harness
int sepol_port_key_extract(sepol_handle_t *handle, const sepol_port_t *port, sepol_port_key_t **key_ptr);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 24) return 0;
    sepol_handle_t *handle = (sepol_handle_t*)calloc(1, sizeof(sepol_handle_t));
    sepol_port_t *port = (sepol_port_t*)calloc(1, sizeof(sepol_port_t));

    // Make fields symbolic before freeing to model attacker-controlled reclamation
    { memcpy(port, fuzz_data + 0, 24); };

    // Free to create stale pointer; subsequent field reads are UAF
    free(port);

    sepol_port_key_t *key = NULL;
    sepol_port_key_extract(handle, port, &key);
    return 0;
}
