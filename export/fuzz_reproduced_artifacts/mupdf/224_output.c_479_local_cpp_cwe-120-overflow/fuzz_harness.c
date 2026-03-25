#include <stdint.h>
#include <stddef.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <string.h>

// Prototype from harness
void fz_write_buffer(fz_context *ctx, fz_output *out, fz_buffer *buf);

int LLVMFuzzerTestOneInput(const uint8_t *fuzz_data, size_t fuzz_size) {
    if (fuzz_size < 528) return 0;
    // Allocate context and objects
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));
    fz_output *out = (fz_output *)calloc(1, sizeof(fz_output));
    fz_buffer *buf = (fz_buffer *)calloc(1, sizeof(fz_buffer));

    // Allocate a SMALL actual buffer, but advertise a LARGER capacity via ep
    const size_t OUT_BUF_ALLOC = 16;   // real allocated size (small)
    const size_t FAKE_CAPACITY = 256;  // logical capacity used for comparisons

    char *out_storage = (char *)malloc(OUT_BUF_ALLOC);
    { memcpy(out_storage, fuzz_data + 0, 16); };

    out->bp = out_storage;
    out->wp = out_storage;                  // start at beginning
    out->ep = out_storage + FAKE_CAPACITY;  // pretend the buffer is bigger than it is
    out->state = NULL;
    out->write = NULL; // not used on targeted path

    // Prepare input buffer
    unsigned char *in_data = (unsigned char *)malloc(512);
    { memcpy(in_data, fuzz_data + 16, 512); };

    size_t sz;
    { static const unsigned char buf_len_data[] = {0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; memcpy(&sz, buf_len_data, (sizeof(sz) < sizeof(buf_len_data)) ? sizeof(sz) : sizeof(buf_len_data)); };
    // Force else-if branch (fits in current buffer) while making memcpy overflow the real allocation:
    //  - sz < (ep - bp) == FAKE_CAPACITY  => take else-if path
    //  - sz > OUT_BUF_ALLOC               => memcpy writes past the allocated object
    /* klee_assume removed */
    /* klee_assume removed */

    buf->data = in_data;
    buf->len = sz;

    // Call entry -> vulnerable memcpy path
    fz_write_buffer(ctx, out, buf);

    return 0;
}
