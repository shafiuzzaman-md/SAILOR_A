#include "harness_types.h"
#include <stdlib.h>
#include <string.h>
// klee removed for replay

int main() {
    // Allocate concrete structs
    fz_context *ctx = (fz_context *)calloc(1, sizeof(fz_context));

    fz_pixmap *pix = (fz_pixmap *)calloc(1, sizeof(fz_pixmap));
    fz_halftone *ht = (fz_halftone *)calloc(1, sizeof(fz_halftone));

    // Allocate buffers with CONCRETE sizes
    // pixmap must be at least 3 bytes to reach the third read safely
    unsigned char *pixbuf = (unsigned char *)malloc(4);
    // ht_line deliberately too short (2 bytes) to trigger OOB on ht_line[2]
    unsigned char *htbuf = (unsigned char *)malloc(2);

    // Make contents symbolic (sizes remain concrete as required)
    { static const unsigned char pixbuf_data[] = {0x3e, 0x3e, 0x3e, 0x3e}; memcpy(pixbuf, pixbuf_data, (4 < sizeof(pixbuf_data)) ? 4 : sizeof(pixbuf_data)); };
    { static const unsigned char htbuf_data[] = {0x40, 0x40}; memcpy(htbuf, htbuf_data, (2 < sizeof(htbuf_data)) ? 2 : sizeof(htbuf_data)); };

    // Wire up structures
    pix->samples = pixbuf;
    ht->threshold = htbuf;

    // Call entry (neutralized pass-through to vulnerable function)
    fz_new_bitmap_from_pixmap(ctx, pix, ht);

    return 0;
}
