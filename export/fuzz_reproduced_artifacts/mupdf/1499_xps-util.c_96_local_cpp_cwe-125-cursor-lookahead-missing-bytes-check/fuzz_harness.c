#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Entry function from harness
extern void xps_resolve_url(fz_context *ctx, xps_document *doc, char *output, char *base_uri, char *path, int output_size);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 1) return 0;
    // Allocate a 1-byte buffer for path so that p[1] is out-of-bounds
    char *path = (char *)malloc(1);
    if (!path) return 0;

    // Make the single byte symbolic and force it to '/'
    { memcpy(path, fuzz_data + 0, 1); };
    /* klee_assume removed */
    // No NUL terminator and no second byte is allocated

    // Unused by our neutralized entry but pass valid pointers
    char outbuf[8]; memset(outbuf, 0, sizeof(outbuf));
    char basebuf[8]; memset(basebuf, 0, sizeof(basebuf));

    xps_resolve_url(NULL, NULL, outbuf, basebuf, path, (int)sizeof(outbuf));
    return 0;
}
