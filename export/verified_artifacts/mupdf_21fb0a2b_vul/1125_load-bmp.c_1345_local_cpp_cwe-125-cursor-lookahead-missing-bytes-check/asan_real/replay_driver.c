#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stddef.h>
#include <stdlib.h>

// Declare entry function
extern struct fz_pixmap *fz_load_bmp(struct fz_context *ctx, const unsigned char *p, size_t total);

int main(void)
{
    // Context object
    struct fz_context ctx_obj; // definition comes from harness_types.h
    { static const unsigned char ctx_obj_data[] = {0x00, 0x00, 0x00, 0x00}; memcpy(&ctx_obj, ctx_obj_data, (sizeof(ctx_obj) < sizeof(ctx_obj_data)) ? sizeof(ctx_obj) : sizeof(ctx_obj_data)); };

    // Allocate a 1-byte buffer so that p[1] is out-of-bounds
    unsigned char *buf = (unsigned char*)malloc(1);
    if (!buf) return 0;
    { static const unsigned char bmp_buf_data[] = {0x00}; memcpy(buf, bmp_buf_data, (1 < sizeof(bmp_buf_data)) ? 1 : sizeof(bmp_buf_data)); };

    // total length: 1 byte ensures (end - p < 14) and p[1] is OOB
    size_t total = 1;

    // Call entry
    (void)fz_load_bmp(&ctx_obj, buf, total);
    return 0;
}
