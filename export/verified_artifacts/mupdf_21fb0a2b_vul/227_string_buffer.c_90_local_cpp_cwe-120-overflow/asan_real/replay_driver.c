#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// Prototypes from harness
int gumbo_string_buffer_to_string(struct GumboInternalParser* parser, struct GumboStringBuffer* input);

int main() {
    // Allocate the GumboStringBuffer container
    struct GumboStringBuffer *input = (struct GumboStringBuffer*)calloc(1, sizeof(*input));

    // Concrete, small source buffer (8 bytes) to induce over-read
    const size_t src_size = 8;
    char *src = (char*)malloc(src_size);
    // Make the source contents symbolic (not strictly necessary for the OOB, but useful)
    { static const unsigned char src_bytes_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(src, src_bytes_data, (src_size < sizeof(src_bytes_data)) ? src_size : sizeof(src_bytes_data)); };

    // Set input fields: length is CONCRETE and larger than src_size to force OOB read in memcpy
    input->data = src;
    input->length = 20; // > src_size, ensures memcpy reads past end of src

    // Parser pointer is unused by gumbo_parser_allocate in the harness; NULL is fine
    struct GumboInternalParser *parser = NULL;

    // Call the entry function (direct pass-through to vulnerable function)
    gumbo_string_buffer_to_string(parser, input);
    return 0;
}
