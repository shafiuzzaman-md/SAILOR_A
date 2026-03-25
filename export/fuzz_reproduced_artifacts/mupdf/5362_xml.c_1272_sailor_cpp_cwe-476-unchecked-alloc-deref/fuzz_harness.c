#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef FZ_XML_MAX_DEPTH
#define FZ_XML_MAX_DEPTH 4096
#endif

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 512) return 0;
    // Allocate concrete context and buffer to satisfy entry preconditions
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_buffer *buf = (fz_buffer *)calloc(1, sizeof(fz_buffer));

    // Allocate a real buffer larger than FZ_XML_MAX_DEPTH and make it symbolic
    const size_t SZ = 8192; // concrete size > 4096
    unsigned char *data = (unsigned char *)malloc(SZ);
    { memcpy(data, fuzz_data + 0, 512); };

    // Set buffer fields (driver uses the struct layout from harness_types.h)
    buf->data = data;
    buf->len = SZ;

    // Call entry directly
    (void)fz_parse_xml_from_html5(ctx, buf);

    return 0;
}
