#include <string.h>
// NO_HARNESS_TYPES
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

// Opaque forward declarations matching harness signatures
typedef struct fz_context fz_context;
typedef struct xps_document xps_document;

// Entry from harness
void xps_resolve_url(fz_context *ctx, xps_document *doc, char *output, char *base_uri, char *path, int output_size);

int main() {
    fz_context *ctx = 0;
    xps_document *doc = 0;

    int output_size = 2; // concrete size
    char *output = (char *)malloc(output_size);
    if (!output) return 1;

    // Make buffer symbolic and force '..' to take the vulnerable branch
    { static const unsigned char output_buf_data[] = {0x2e, 0x2e}; memcpy(output, output_buf_data, (output_size < sizeof(output_buf_data)) ? output_size : sizeof(output_buf_data)); };
    /* klee_assume removed */
    /* klee_assume removed */

    char *base_uri = 0;
    char *path = 0;

    xps_resolve_url(ctx, doc, output, base_uri, path, output_size);
    return 0;
}
