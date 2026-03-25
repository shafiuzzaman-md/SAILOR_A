#include <string.h>
#include "harness_types.h"
// klee removed for replay
#include <stdlib.h>
#include <stdint.h>

int main() {
    // Context is unused by our neutralized harness; pass NULL
    fz_context *ctx = (fz_context*)0;

    // Allocate buffer struct
    fz_buffer *buffer = (fz_buffer*)calloc(1, sizeof(fz_buffer));

    // Allocate a 1-byte data buffer so access to p[1] is OOB when p[0] == 'P'
    size_t sz = 1;
    unsigned char *data = (unsigned char*)malloc(sz);

    // Make the single byte symbolic so KLEE can choose 'P' and trigger the OOB read of p[1]
    { static const unsigned char imgbuf_data[] = {0x50}; memcpy(data, imgbuf_data, (sz < sizeof(imgbuf_data)) ? sz : sizeof(imgbuf_data)); };

    buffer->len = sz;
    buffer->data = data;

    // Call entry function (neutralized pass-through to vulnerable function)
    fz_new_image_from_buffer(ctx, buffer);

    return 0;
}
