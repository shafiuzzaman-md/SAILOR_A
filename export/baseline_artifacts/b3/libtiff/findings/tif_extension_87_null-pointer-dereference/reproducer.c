#include <tiffio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    // Create a temporary filename in /tmp
    char tmpl[] = "/tmp/libtiff_clientinfo_npd_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0) {
        perror("mkstemp");
        return 1;
    }
    close(fd);

    // Open a TIFF handle in write mode so we get a valid TIFF* structure
    TIFF *tif = TIFFOpen(tmpl, "w");
    if (!tif) {
        fprintf(stderr, "TIFFOpen failed\n");
        return 1;
    }

    // First, set a client info entry with a non-NULL name to create the list
    int dummy1 = 42;
    TIFFSetClientInfo(tif, &dummy1, "first-entry");

    // Now trigger the bug: pass name == NULL while tif->tif_clientinfo is non-NULL
    // This causes strcmp(psLink->name, name) to dereference a NULL pointer
    int dummy2 = 7;
    TIFFSetClientInfo(tif, &dummy2, NULL);

    // Not reached if the bug is present
    TIFFClose(tif);
    unlink(tmpl);
    return 0;
}
