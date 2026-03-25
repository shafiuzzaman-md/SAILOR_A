#include <stddef.h>
#include <string.h>
// harness/driver.c
#include "harness_types.h"
// klee removed for replay
#include <stdint.h>
#include <stdlib.h>

// Entry prototype from spine
void fz_load_bmp_info(fz_context *ctx, const unsigned char *p, size_t total, int *wp, int *hp, int *xresp, int *yresp, fz_colorspace **cspacep);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 1) return 0;
    // Allocate minimal context
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));

    // Allocate a tiny buffer (1 byte) and make its content symbolic
    unsigned char *buf = (unsigned char *)malloc(1);
    { memcpy(buf, fuzz_data + 0, 1); };

    // Make length symbolic but small to satisfy end - p < 14 and trigger p[1] OOB
    size_t len;
    { static const unsigned char len_data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&len, len_data, (sizeof(len) < sizeof(len_data)) ? sizeof(len) : sizeof(len_data)); };
    /* klee_assume removed */ // 0 or 1 bytes available → p[1] is out-of-bounds

    int w = 0, h = 0, xr = 0, yr = 0;
    fz_colorspace *cs = 0;

    fz_load_bmp_info(ctx, buf, len, &w, &h, &xr, &yr, &cs);
    return 0;
}
