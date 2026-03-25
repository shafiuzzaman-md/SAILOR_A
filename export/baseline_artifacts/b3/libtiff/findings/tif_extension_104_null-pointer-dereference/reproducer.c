#include <tiffio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    // Create a temporary writable file path for TIFFOpen
    char tmpl[] = "/tmp/libtiff_clientinfo_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0) {
        perror("mkstemp");
        return 1;
    }
    close(fd);

    // Open a fresh TIFF handle; tif->tif_clientinfo should be NULL initially
    TIFF *tif = TIFFOpen(tmpl, "w");
    if (!tif) {
        fprintf(stderr, "TIFFOpen failed\n");
        unlink(tmpl);
        return 1;
    }

    // Trigger: pass name == NULL when no existing clientinfo link exists
    // This reaches strlen(name) in TIFFSetClientInfo (tif_extension.c:104)
    void *data = (void*)0xDEADBEEF;
    const char *name = NULL;  // Vulnerable input
    TIFFSetClientInfo(tif, data, name);

    // We should never reach here due to the null-pointer dereference
    TIFFClose(tif);
    unlink(tmpl);
    return 0;
}
