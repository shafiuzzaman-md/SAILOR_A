#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

int sepol_bool_key_extract(sepol_handle_t *handle, const sepol_bool_t *boolean, sepol_bool_key_t **key_ptr);

int main() {
    // Allocate handle concretely
    sepol_handle_t *handle = (sepol_handle_t *)calloc(1, sizeof(sepol_handle_t));

    // Allocate a sepol_bool_t and its name buffer
    sepol_bool_t *b = (sepol_bool_t *)malloc(sizeof(sepol_bool_t));
    char *namebuf = (char *)malloc(32);
    { static const unsigned char namebuf_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(namebuf, namebuf_data, (32 < sizeof(namebuf_data)) ? 32 : sizeof(namebuf_data)); };
    // Ensure NUL-termination to satisfy potential string ops
    namebuf[31] = '\0';

    // Initialize fields
    b->name = namebuf;
    b->value = 0;

    // Create a stale pointer scenario: free the sepol_bool_t but keep the pointer
    sepol_bool_t *stale_b = b;
    free(b);  // UAF: stale_b now points to freed memory

    // Prepare out parameter
    sepol_bool_key_t *out_key = NULL;

    // Call entry (neutralized pass-through to vulnerable function)
    sepol_bool_key_extract(handle, stale_b, &out_key);

    return 0;
}
