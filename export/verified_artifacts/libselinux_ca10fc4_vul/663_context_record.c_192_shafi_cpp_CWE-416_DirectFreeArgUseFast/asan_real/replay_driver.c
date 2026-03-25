#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Entry prototype from harness
int sepol_context_free(sepol_context_t *con);

int main() {
    // Allocate concrete context
    sepol_context_t *con = (sepol_context_t *)calloc(1, sizeof(sepol_context_t));

    // Allocate one concrete buffer and alias two fields to it to cause double-free
    char *shared = (char *)malloc(16);
    if (!shared) return 0;
    { static const unsigned char shared_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(shared, shared_bytes_data, (16 < sizeof(shared_bytes_data)) ? 16 : sizeof(shared_bytes_data)); };

    con->user = shared;     // first free
    con->type = shared;     // second free of same pointer -> double-free at free(con->type)

    // Other fields safe (NULL is fine for free)
    con->role = NULL;
    con->mls  = NULL;

    // Call pass-through entry
    sepol_context_free(con);
    return 0;
}
